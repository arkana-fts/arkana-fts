/**
 * \file vector.cpp
 * \author Pompei2
 * \date 1 February 2010
 * \brief This file contains the implementation of the vector class. This class defines a 3-component vector.
 **/

#include "Vector.h"
#include "Matrix.h"

#include "main/Exception.h"
#include "dLib/dString/dString.h"
#include "dLib/dFile/dFile.h"

using namespace FTS;

////////////////////////////////////////////
// Constructors and assignment operators. //
////////////////////////////////////////////

FTS::Vector::Vector()
    : m_v(new float[4])
{
    m_v[0] = 0.0f;
    m_v[1] = 0.0f;
    m_v[2] = 0.0f;
    m_v[3] = 1.0f;
}

FTS::Vector::Vector(float in_fVal)
    : m_v(new float[4])
{
    m_v[0] = in_fVal;
    m_v[1] = in_fVal;
    m_v[2] = in_fVal;
    m_v[3] = 1.0f;
}

FTS::Vector::Vector(const float in_v[3])
    : m_v(new float[4])
{
    m_v[0] = in_v[0];
    m_v[1] = in_v[1];
    m_v[2] = in_v[2];
    m_v[3] = 1.0f;
}

FTS::Vector::Vector(const Vector& in_v)
    : m_v(new float[4])
{
    m_v[0] = in_v.x();
    m_v[1] = in_v.y();
    m_v[2] = in_v.z();
    m_v[3] = 1.0f;
}

const Vector& FTS::Vector::operator=(const Vector& in_v)
{
    m_v[0] = in_v.x();
    m_v[1] = in_v.y();
    m_v[2] = in_v.z();
    m_v[3] = 1.0f;

    return *this;
}

FTS::Vector::Vector(float in_fX, float in_fY, float in_fZ)
    : m_v(new float[4])
{
    m_v[0] = in_fX;
    m_v[1] = in_fY;
    m_v[2] = in_fZ;
    m_v[3] = 1.0f;
}

FTS::Vector::Vector(float in_fX, float in_fY, float in_fZ, float in_fW)
    : m_v(new float[4])
{
    if(nearZero(in_fW)) {
        m_v[0] = in_fX;
        m_v[1] = in_fY;
        m_v[2] = in_fZ;
        m_v[3] = 0.0f;
    } else {
        m_v[0] = in_fX/in_fW;
        m_v[1] = in_fY/in_fW;
        m_v[2] = in_fZ/in_fW;
        m_v[3] = 1.0f;
    }
}

FTS::Vector::Vector(const Vector& in_v, float in_fW)
    : m_v(new float[4])
{
    if(nearZero(in_fW)) {
        m_v[0] = in_v.x();
        m_v[1] = in_v.y();
        m_v[2] = in_v.z();
        m_v[3] = 0.0f;
    } else {
        m_v[0] = in_v.x()/in_fW;
        m_v[1] = in_v.y()/in_fW;
        m_v[2] = in_v.z()/in_fW;
        m_v[3] = 1.0f;
    }
}

FTS::Vector::Vector(Vector&& in_v)
{
    this->operator=(std::move(in_v));
}

const Vector& FTS::Vector::operator=(Vector&& in_v)
{
    m_v = in_v.m_v;
    in_v.m_v = nullptr;

    return *this;
}

Vector::~Vector()
{
    if(m_v) delete [] m_v;
}

///////////////////////////////////////
// Conversion methods and operators. //
///////////////////////////////////////

String FTS::Vector::toString(unsigned int in_iDecimalPlaces) const
{
    return "("+String::nr(this->x(), in_iDecimalPlaces)+", "
              +String::nr(this->y(), in_iDecimalPlaces)+", "
              +String::nr(this->z(), in_iDecimalPlaces)+")";
}

FTS::Vector::operator String() const
{
    return this->toString();
}

std::ostream& operator<< (std::ostream& os, const FTS::Vector& me)
{
    return os << me.toString();
}

#ifdef D_CAL3D_INTEGRATION
FTS::Vector::Vector(const CalVector& o)
    : m_v(new float[4])
{
    m_v[0] = o.x;
    m_v[1] = o.y;
    m_v[2] = o.z;
    m_v[3] = 1.0f;
}

FTS::Vector::operator CalVector() const
{
    return CalVector(this->x(), this->y(), this->z());
}
#endif

/////////////////////////////////////
// Accessors, getters and setters. //
/////////////////////////////////////

float& FTS::Vector::operator[](unsigned int idx)
{
    if(idx > 3)
        throw FTS::NotExistException("Index " + String::nr(idx), "Vector");

    return m_v[idx];
}

float FTS::Vector::operator[](unsigned int idx) const
{
    if(idx > 3)
        throw FTS::NotExistException("Index " + String::nr(idx), "Vector");

    return m_v[idx];
}

////////////////////////////////
// Basic Vector calculations. //
////////////////////////////////

Vector FTS::Vector::operator -() const
{
    return Vector(-this->x(),
                  -this->y(),
                  -this->z());
}

Vector FTS::Vector::operator +(const Vector& in_v) const
{
    return Vector(this->x() + in_v.x(),
                  this->y() + in_v.y(),
                  this->z() + in_v.z());
}

Vector FTS::Vector::operator -(const Vector& in_v) const
{
    return (*this) + (-in_v);
}

Vector FTS::Vector::operator *(float in_f) const
{
    return Vector(this->x() * in_f,
                  this->y() * in_f,
                  this->z() * in_f);
}

void FTS::Vector::operator +=(const Vector& in_v)
{
    this->operator=((*this) + in_v);
}

void FTS::Vector::operator -=(const Vector& in_v)
{
    this->operator+=(-in_v);
}

void FTS::Vector::operator *=(float in_f)
{
    this->operator=((*this) * in_f);
}

Vector FTS::Vector::cross(const Vector& in_v) const
{
    return Vector(this->y()*in_v.z() - this->z()*in_v.y(),
                  this->z()*in_v.x() - this->x()*in_v.z(),
                  this->x()*in_v.y() - this->y()*in_v.x());
}

float FTS::Vector::dot(const Vector& in_v) const
{
    return this->x()*in_v.x()
         + this->y()*in_v.y()
         + this->z()*in_v.z();
}

///////////////////////////////////////
// Vector length related operations. //
///////////////////////////////////////

float FTS::Vector::len() const
{
    return sqrt(this->x()*this->x()
              + this->y()*this->y()
              + this->z()*this->z());
}

Vector& FTS::Vector::normalize()
{
    // The zero-vector stays the zero-vector.
    if(nearZero(this->x()) &&
       nearZero(this->y()) &&
       nearZero(this->z()) ) {
        return this->setX(0.0f).setY(0.0f).setZ(0.0f);
    }

    float l = this->len();

    // Very little vectors will be stretched to a unit vector in one direction.
    if(nearZero(l)) {
        if((this->x() >= this->y())
        && (this->x() >= this->z())
        && (this->x() >= 0.0f)) {
            return this->setX(1.0f).setY(0.0f).setZ(0.0f);
        } else if((this->x() <= this->y())
               && (this->x() <= this->z())
               && (this->x() <= 0.0f)) {
            return this->setX(-1.0f).setY(0.0f).setZ(0.0f);
        } else {
            if(this->y() >= this->z()
            && this->y() >= 0.0f) {
                return this->setX(0.0f).setY(1.0f).setZ(0.0f);
            } else if(this->y() <= this->z()
                   && this->y() <= 0.0f) {
                return this->setX(0.0f).setY(-1.0f).setZ(0.0f);
            } else {
                return this->setX(0.0f).setY(0.0f).setZ(this->z() >= 0.0f ? 1.0f : -1.0f);
            }
        }
    } else {
        // Follows the usual normalization rule.
        float m = 1.0f / l;
        return this->setX(this->x()*m).setY(this->y()*m).setZ(this->z()*m);
    }
}

Vector FTS::Vector::normalized() const
{
    Vector copy(*this);
    return copy.normalize();
}

//////////////////////////////////////
// Vector interpolation operations. //
//////////////////////////////////////

Vector FTS::Vector::lerp(const Vector& v2, float between) const
{
    return *this + (v2 - *this)*between;
}

///////////////////////////
// Vector serialization. //
///////////////////////////

File& FTS::operator<<(File& f, const Vector& v) {
    return f << v.x() << v.y() << v.z();
}

File& FTS::operator>>(File& f, Vector& v) {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    f >> x >> y >> z;
    v.setX(x).setY(y).setZ(z);
    return f;
}

////////////////////////////
// Vector transformation. //
////////////////////////////

Vector FTS::operator*(const Base4x4Matrix& m, const Vector& v)
{
    return Vector(m[0]*v[0] + m[4]*v[1] + m[8] *v[2] + m[12]*v[3],
                  m[1]*v[0] + m[5]*v[1] + m[9] *v[2] + m[13]*v[3],
                  m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]*v[3],
                  m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]*v[3]);
}
