#include "server_log.h"
#include "db.h"
#include "client.h"
#include "channel.h"
#include "game.h"
#include "utilities/threading.h"
#include "net/connection.h"
#include "socket_connection_waiter.h"
#include "utilities/GetOpt.h"

#include "constants.h"

#include "server.h" // Move here to avoid including problems w/ main.h in net/connection.h 

#if WINDOOF
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <list>
#include <chrono>
#include <thread>
#include <algorithm>
#include <signal.h>
#if !WINDOOF
#include <pwd.h>
#endif

/* Change this to whatever your daemon is called */
#define DAEMON_NAME "fts-server"

/* The name of the lockfile, directory will be home at runtime. */
#define LOCK_FILE DAEMON_NAME ".lock"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

bool g_bExit = false;
bool stopSpamThread = false;

void connectionListener(void *in_iPort);
void help( const std::string& in_pszLine, char *in_pszMe );
static void daemonize( const char *lockfile, const char *dir );
static void trytokill(const char *lockfile);

using namespace std;
using namespace FTSSrv2;
using namespace FTS;

void testSpammer(void *args)
{
    Channel *pChan = (Channel *)args;

    FTSMSGDBG("Starting the test spammer in default channel.", 1);

    Packet p(DSRV_MSG_CHAT_GETMSG);
    p.append(DSRV_CHAT_TYPE_NORMAL);
    p.append((int8_t)0);
    p.append(String("Test_Spammer"));
    p.append(String("spam0r messag0r"));

    // wait for connections or quit.
    while(!g_bExit && !stopSpamThread) {
        std::this_thread::sleep_for( std::chrono::microseconds ( 100 ) );
        pChan->sendPacketToAll(&p);
    }

}

std::string getNextToken( stringstream& sb, char delimiter = ' ' )
{
    string token;
    do {
        getline( sb, token, delimiter );
    } while( token.empty() && !sb.eof() );

    return token;
}

int main(int argc, char *argv[])
{
#ifdef _DEBUG && WINDOOF
    int flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    flag |= _CRTDBG_LEAK_CHECK_DF; // Turn on leak-checking bit
    _CrtSetDbgFlag( flag );
    //_CrtSetBreakAlloc( 5382 ); // Comment or un-comment on need basis
#endif

    bool bDaemon = false, bVerbose = false;
    String logdir(DSRV_LOG_DIR);
    int opt = -1;

#if WINDOOF
    //----------------------
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( iResult != NO_ERROR )
    {
        fprintf(stdout, "WSAStartup failed with error: %ld\n", iResult );
        return 1;
    }
#endif

#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN); /* Ignore broken pipe */
#endif /* SIGPIPE */

    String sHome = getenv("HOME");
    bool bJustKill = false;
    GetOpt getopt( argc, argv, "H:l:vdkh" );
    while((opt = getopt()) != -1) {
        switch(opt) {
        case 'H':
            sHome = String(getopt.get());
            break;
        case 'l':
            logdir = String(getopt.get());
            break;
        case 'v':
            bVerbose = true;
            break;
        case 'd':
            bDaemon = true;
            break;
        case 'k':
            bJustKill = true;
            break;
        case 'h':
        default:
            std::cout << "usage: " << " [-H HOMEDIR] [-l LOGDIR] [-v] [-h]\n" ;
            std::cout << "      -H HOMEDIR assume the home directory to be HOMEDIR.\n"
                      << "                  The lockfile will be written there.\n";
            std::cout << "      -l LOGDIR  sets the directory to write logfiles\n";
            std::cout << "      -v         activates verbose mode\n";
#if !WINDOOF
            std::cout << "      -d         start as a daemon (not interactive) (EXPERIMENTAL)\n";
#endif
            std::cout << "      -k         shut down the active daemon (TODO)\n";
            std::cout << "      -h         shows this help message\n";
            exit(EXIT_SUCCESS);
            break;
        }
    }

    String sLockFile = sHome + "/" + LOCK_FILE;

    if(bJustKill) {
        trytokill(sLockFile.c_str());
        exit(EXIT_SUCCESS);
    }

    // Lockfile checking to start only once.
    // =====================================
    int lfp = -1;

    // Check the lockfile
    lfp = open(sLockFile.c_str(),O_RDONLY);

    if(lfp >= 0) {
        std::cout << "A lockfile already exists, this usually means that"
               " the server is already started. Please first stop the"
               " server using the -k switch or delete the lockfile if"
               " you are sure the server is not running.\n";
        exit(EXIT_FAILURE);
    } 

    // Create the lockfile.
    lfp = open(sLockFile.c_str(),O_RDWR|O_CREAT,0640);
    if ( lfp < 0 ) {
        std::cerr << "unable to create lock file " << sLockFile << " (" << strerror(errno) << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
    close( lfp );

    // Logging and daemonizing.
    // ========================
    new FTSSrv2::ServerLogger(logdir, bVerbose);

    // Daemonize if wanted.
    if(bDaemon)
        daemonize(sLockFile.c_str(), sHome.c_str());

    // Database and time init.
    // =======================
    FTSTools::init();

    // Try to connect to the mysql database.
    if(ERR_OK != DataBase::initUniqueDB()) {
        // We still need to remove the lockfile.
        FTSMSGDBG("Removing the lockfile "+sLockFile+".\n", 1);
        if(0 != remove(sLockFile.c_str())) {
            FTSMSG("Error removing the lockfile "+sLockFile+
                   ". If the file still exists, you should try to remove "
                   "it by hand, so the server will start next time.\n", MsgType::Error);
        }

        FTSMSGDBG("Everything done, bye\n", 1);
        delete FTS::Logger::getSingletonPtr();

        return -1;
    }

    ChannelManager::init();
    ClientsManager::init();
    GameManager::init();

    // Begin to listen on all ports.
    // =============================

    // Create a thread waiting on each port. Including the fts port: 0xAF75 :)
    // DEBUG: the +1 at the end.
    std::thread threads[DSRV_PORT_LAST + 1 - DSRV_PORT_FIRST + 1];

    for(size_t i = DSRV_PORT_FIRST; i < DSRV_PORT_LAST + 1; i++)
        threads[i - DSRV_PORT_FIRST] = std::thread(connectionListener, (void *)i) ;

    // DEBUG: a test spamming thread.
    std::thread tSpamThread;

    // Here we access directly to it, because we just read it, so there's no danger (I hope)
    while(!g_bExit) {
        // Don't read stdin if being a daemon.
        if(bDaemon) {
            std::this_thread::sleep_for( std::chrono::microseconds(10));
            continue;
        }

        srvFlush(stdout);

        string s;
        getline( cin, s );
        auto linebuf = s.erase( 0, s.find_first_not_of( " \t\n" ) );
        stringstream sb( linebuf );
        string cmd;

        while ( getline( sb, cmd, ' ' ) )
        {
            std::transform( cmd.begin(), cmd.end(), cmd.begin(), ::tolower ); // Thanks to SO
            // Parse it !
            if( cmd == "help"  ) {
                auto arg = getNextToken( sb );
                std::transform( arg.begin(), arg.end(), arg.begin(), ::tolower ); // Thanks to SO
                help( arg, argv[0] );
            } else if( cmd == "exit" ) {
                g_bExit = true;
                break;
            } else if( cmd == "nplayers" ) {
                size_t nPlayers = dynamic_cast< ServerLogger * >(FTS::Logger::getSingletonPtr())->getPlayerCount();
                FTSMSGDBG( "Number of players that are logged in: " + String::nr( nPlayers ), 1 );
            } else if( cmd == "ngames" ) {
                size_t nGames = dynamic_cast< ServerLogger * >(FTS::Logger::getSingletonPtr())->getGameCount();
                FTSMSGDBG( "Number of games that are opened: " + String::nr( nGames ), 1 );
            } else if( cmd == "version" ) {
                FTSMSGDBG( "The version of the server is " D_SERVER_VERSION_STR, 1 );
            } else if( cmd == "spam" ) {
                auto arg = getNextToken( sb );
                std::transform( arg.begin(), arg.end(), arg.begin(), ::tolower ); // Thanks to SO
                if( (arg == "start") && !tSpamThread.joinable() ) {
                    tSpamThread = std::thread( testSpammer, ( void * ) ChannelManager::getManager()->getDefaultChannel() );
                    if( tSpamThread.joinable() )
                        FTSMSGDBG( "Spam bot started.", 1 );
                    else
                        FTSMSG( "Spam bot could not be started (" + String( strerror( errno ) ) + ").", MsgType::Error );
                } else if( ( arg == "stop" ) && tSpamThread.joinable() ) {
                    stopSpamThread = true;
                    tSpamThread.join();
                    FTSMSGDBG( "Spam bot stopped.", 1 );
                }
            } else if( cmd == "verbose" ) {
                auto arg = getNextToken( sb );
                std::transform( arg.begin(), arg.end(), arg.begin(), ::tolower ); // Thanks to SO
                if( arg == "on" ) {
                    bool bOld = dynamic_cast< ServerLogger * >(FTS::Logger::getSingletonPtr())->setVerbose( true );
                    FTSMSGDBG( "Verbose mode was " + String( bOld ? "on" : "off" ) + ", now it is on.", 1 );
                } else if( arg == "off" ) {
                    bool bOld = dynamic_cast< ServerLogger * >(FTS::Logger::getSingletonPtr())->setVerbose( false );
                    FTSMSGDBG( "Verbose mode was " + String( bOld ? "on" : "off" ) + ", now it is off.", 1 );
                    FTSMSG( "Verbose mode was " + String( bOld ? "on" : "off" ) + ", now it is off.", MsgType::Message );
                } else {
                    bool b = dynamic_cast< ServerLogger * >(FTS::Logger::getSingletonPtr())->getVerbose();
                    FTSMSG( "Verbose mode is currently " + String( b ? "on" : "off" ), MsgType::Message );
                }
            } else {
                FTSMSG( "Unknown command '" + String( cmd ) + "', u n00b, try typing 'help' to get some help.", MsgType::Error );
            }
        }
    }

    // Now all is lost, nobody likes fts anymore :( clean up.

    FTSMSGDBG("Waiting for every thread to close ...", 1);
    for(size_t i = DSRV_PORT_FIRST; i < DSRV_PORT_LAST + 1; i++) {
        threads[i - DSRV_PORT_FIRST].join();
        FTSMSGDBG("Thread on port 0x"+String::nr(i, -1,'0',std::ios::hex)+" successfully closed.", 1);
    }

    // Shutdown all connectons to all clients that still exist.
    FTSMSGDBG("All threads successfully closed, waiting for all clients to shutdown.", 1);
    ClientsManager::deinit();

    FTSMSGDBG("All clients successfully shot down, waiting for all games to shutdown.", 1);
    GameManager::deinit();

    FTSMSGDBG("All games successfully shot down, waiting for all channels to shutdown.", 1);
    ChannelManager::deinit();
    FTSMSGDBG("All channels successfully shut down.", 1);

    DataBase::deinitUniqueDB();

    // We still need to remove the lockfile.
    FTSMSGDBG("Removing the lockfile "+sLockFile+".\n", 1);
    if(0 != remove(sLockFile.c_str())) {
        FTSMSG("Error removing the lockfile "+sLockFile+
               ". If the file still exists, you should try to remove "
               "it by hand, so the server will start next time.\n", MsgType::Error);
        FTSMSG( "The Error Text is " + String( strerror( errno ) ), MsgType::Error );
    }

    FTSMSGDBG("Everything done, bye\n", 1);
    delete FTS::Logger::getSingletonPtr();

    return EXIT_SUCCESS;
}

// Display some help.
void help(const string& topic, char *in_pszMe)
{

    if(topic == "help") {
        std::cout << "Just type help once, It won't help to type help more often :p\n";
    } else if(topic == "exit") {
        std::cout << "SYNTAX:\n";
        std::cout << "\texit\n";
        std::cout << "\n";
        std::cout << "DESC:\n";
        std::cout << "\tThis will close the connection to all clients and shutdown the server.\n";
        std::cout << "Normally, every client should get a warning message that the server has been shutdown.\n";
        std::cout << "\n";
    } else if(topic == "nplayers") {
        std::cout << "SYNTAX:\n";
        std::cout << "\tnplayers\n";
        std::cout << "\n";
        std::cout << "DESC:\n";
        std::cout << "\tThis just prints out how much players are actually connected.\n";
        std::cout << "The players that are connected, but not logged in are also counted here.\n";
        std::cout << "You can always see this in the file " DSRV_FILE_NPLAYERS".\n";
        std::cout << "\n";
    } else if(topic == "ngames") {
        std::cout << "SYNTAX:\n";
        std::cout << "\tngames\n";
        std::cout << "\n";
        std::cout << "DESC:\n";
        std::cout << "\tThis just prints out how much games are actually opened.\n";
        std::cout << "The players that are opened, but not started yet are also counted here.\n";
        std::cout << "You can always see this in the file " DSRV_FILE_NGAMES".\n";
        std::cout << "\n";
    } else if(topic == "version") {
        std::cout << "SYNTAX:\n";
        std::cout << "\tversion\n";
        std::cout << "\n";
        std::cout << "DESC:\n";
        std::cout << "\tThis just prints out the version of the server.\n";
    } else if(topic == "spam") {
        std::cout << "SYNTAX:\n";
        std::cout << "\tspam [start|stop]\n";
        std::cout << "\n";
        std::cout << "DESC:\n";
        std::cout << "\tThis starts or stops a spam bot, depending on the argument.\n";
        std::cout << "The spam bot will send a spam message 10 times a second in the\n";
        std::cout << "main channel, but he won't appear in the players list and won't\n";
        std::cout << "answer to any message, he just spams messages in the channel.\n";
        std::cout << "\n";
    } else if(topic == "verbose") {
        std::cout << "SYNTAX:\n";
        std::cout << "\tverbose [on|off]\n";
        std::cout << "\n";
        std::cout << "DESC:\n";
        std::cout << "\tThis changes the verbosity mode of the server.\n";
        std::cout << "If verbosity is off, the server only tells you about the errors\n";
        std::cout << "and loggs all the rest into the logfile. If verbosity is on,\n";
        std::cout << "the server tells you everything that happens too.\n";
        std::cout << "If no argument is given, it prints the current verbosity state.\n";
        std::cout << "\n";
    } else {
        std::cout << "Type: help [command], where command is one of the followings:\n";
        std::cout << "\n";
        std::cout << "  exit     shuts down this server.\n";
        std::cout << "  version  see the server-version.\n";
        std::cout << "  nplayers see the number of players.\n";
        std::cout << "  ngames   see the number of games.\n";
        std::cout << "  spam     start/stop a spam bot in the main channel.\n";
        std::cout << "  verbose  let me talk much or not.\n";
        std::cout << "\n";
        std::cout << "FILES:\n";
        std::cout << "All files are located in the current working directory.\n";
        std::cout << "  " << dynamic_cast<ServerLogger *>(FTS::Logger::getSingletonPtr())->getLogfilename() << " contains a lot of logging messages." << std::endl;
        std::cout << "  " << dynamic_cast<ServerLogger *>(FTS::Logger::getSingletonPtr())->getErrfilename() << " contains all error messages that happened." << std::endl;
        std::cout << "  " << dynamic_cast<ServerLogger *>(FTS::Logger::getSingletonPtr())->getPlayersfilename() << " contains the number of players actually connected." << std::endl;
        std::cout << "  " << dynamic_cast<ServerLogger *>(FTS::Logger::getSingletonPtr())->getGamesfilename() << " contains the number of games actually opened." << std::endl;
    }
    srvFlush(stdout);
}

// This sets up everything to listen on a certain port, and then goes listen.
void connectionListener(void *in_iPort)
{
    std::unique_ptr<ConnectionWaiter> pWaiter( new SocketConnectionWaiter );

    if(ERR_OK != pWaiter->init((uint16_t)((size_t)in_iPort)))
        return ;


    // wait for connections unless we need to quit.
    while(!g_bExit) {

        // Wait for a connection&packet for 1000 ms, if none is got,
        // wait a bit to avoid megaload of cpu. 100 microsec = 0.1 millisec.
        pWaiter->waitForThenDoConnection(1000);
        std::this_thread::sleep_for( std::chrono::microseconds(100) );
    }

}

static void child_handler(int signum)
{
#if !WINDOOF
    switch(signum) {
    case SIGALRM: exit(EXIT_FAILURE); break;
    case SIGUSR1: exit(EXIT_SUCCESS); break;
    case SIGUSR2: g_bExit = true; break;
    case SIGCHLD: exit(EXIT_FAILURE); break;
    }
#endif
}

// From: http://www-theorie.physik.unizh.ch/~dpotter/howto/daemonize
static void daemonize( const char *lockfile, const char *dir )
{
#if !WINDOOF
    pid_t pid, sid, parent;

    /* already a daemon */
    if ( getppid() == 1 ) {
        FTSMSG("Already a daemon?");
        return;
    }

    /* Drop user if there is one, and we were run as root */
    if ( getuid() == 0 || geteuid() == 0 ) {
        struct passwd *pw = getpwnam(DSRV_ROOT_IS_BAD);
        if ( pw ) {
            FTSMSGDBG("setting user to " DSRV_ROOT_IS_BAD "\n", 1);
            setuid( pw->pw_uid );
        }
    }

    /* Trap signals that we expect to recieve */
    signal(SIGCHLD,child_handler);
    signal(SIGUSR1,child_handler);
    signal(SIGUSR2,child_handler);
    signal(SIGALRM,child_handler);

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        FTSMSG("unable to fork daemon, ("+String(strerror(errno))+")\n", MsgType::Error);
        exit(EXIT_FAILURE);
    }

    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {

        /* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
           for two seconds to elapse (SIGALRM).  pause() should not return. */
        alarm(2);
        pause();

        exit(EXIT_FAILURE);
    }

    /* At this point we are executing as the child process */
    parent = getppid();

    /* Cancel certain signals */
    signal(SIGCHLD,SIG_DFL); /* A child process dies */
    signal(SIGTSTP,SIG_IGN); /* Various TTY signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */
    signal(SIGTERM,SIG_DFL); /* Die on SIGTERM */

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        FTSMSGDBG("unable to create a new session, ("+String(strerror(errno))+")\n", MsgType::Error);
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory.  This prevents the current
       directory from being locked; hence not being able to remove it. */
    if ((chdir(dir)) < 0) {
        FTSMSGDBG("unable to change directory to " + String(dir) + ", ("+String(strerror(errno))+")\n", MsgType::Error);
        exit(EXIT_FAILURE);
    }

    /* Write the pid to the lockfile. */
    FILE *pFile = fopen(lockfile, "w+");
    fprintf(pFile, "%d", getpid());
    fflush(pFile);
    fclose(pFile);

    /* Redirect standard files to /dev/null */
    dynamic_cast<ServerLogger *>(FTS::Logger::getSingletonPtr())->daemonized();
    freopen( "/dev/null", "r", stdin);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);

    /* Tell the parent process that we are A-okay */
    kill( parent, SIGUSR1 );
#endif
}

static void trytokill(const char *lockfile)
{
#if !WINDOOF
    if(lockfile == NULL || lockfile[0] == '\0')
        return;

    // Get the PID of the server.
    int pid;
    FILE *pFile = fopen(lockfile, "r");
    if(!pFile) {
        printf("Could not stop the server, as the lockfile %s does not exist !"
               " Bye.\n", lockfile);
        printf("This could also mean that the server is not running.\n");
        return ;
    }
    if(EOF == fscanf(pFile, "%d", &pid)) {
        printf("The lockfile %s is empty (does not contain the pid) ! Just deleting it.\n", lockfile);
        fclose(pFile);
        if(0 != remove(lockfile)) {
            printf("Removing the lockfile did not work, thus you have a problem now.\n");
        } else {
            printf("Removing the lockfile did work. Everything should be fine now.\n");
        }
        return ;
    }
    fclose(pFile);

    printf("The server (PID: %d) is shutting down, waiting for it to be done ... \n", pid);
    fflush(stdout);

    // Tell the server to quit.
    if(0 != kill(pid, SIGUSR2)) {
        if(errno == ESRCH) {
            printf("There is one problem, I see two reasons for it:\n");
            printf("  * Either the daemon is no more running but the lockfile\n"
                   "    %s still exists.\n"
                   "    I Will try to delete it, if that succeeds, everything is ok.\n", lockfile);
            printf("  * Or either the daemon has changed its PID (very strange)\n"
                   "    or the lockfile %s has been modified.\n"
                   "    You should check if the daemon is still running.\n"
                   "    by typing \"ps aux | grep fts\". And kill it manually\n"
                   "    by typing \"kill -12 [THE PID]\". Good Luck !\n", lockfile);
            printf("Trying to remove the lockfile anyway ...\n");
            if(0 != remove(lockfile)) {
                printf("Removing the lockfile did not work, thus you have a problem now.\n");
            } else {
                printf("Removing the lockfile did work. Everything should be fine now (hopefully).\n");
            }
        } else {
            printf("Could not stop the server: %d: %s\n", errno, strerror(errno));
            // Here, we don't remove the lockfile, as the server maybe could not be killed
            // because of permissions and is most likely to still run.
        }
    }

    // Give the server some time ...
    sleep(5);

    printf("If you see any numbers below, the server is still running ! Maybe you just need to give it more time, check ps | grep " DAEMON_NAME " in a few seconds again.\n");
//    system(("cat "+String(lockfile)+" 2> /dev/null").c_str());
//    system("ps -C "DAEMON_NAME" -o pid,cmd=");
    system("ps | grep " DAEMON_NAME );
    printf("\nIf you didn't see any number one line above, the server has quit successfully.\n");
#endif
}
