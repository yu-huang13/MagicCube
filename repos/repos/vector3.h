#ifndef VECTOR3_H
#define VECTOR3_H

#include <iostream>
using namespace std;

struct Vector3
{
	double x, y, z;
	Vector3(double xx = 0, double yy = 0, double zz = 0);
	Vector3(const Vector3 &v);

	friend Vector3 operator - (const Vector3 &A);

	friend Vector3 operator + (const Vector3 &A, const Vector3 &B);
	friend Vector3 operator - (const Vector3 &A, const Vector3 &B);
	friend Vector3 operator * (const Vector3 &A, const double &num);
	friend Vector3 operator / (const Vector3 &A, const double &num);
	friend Vector3 operator * (const Vector3 &A, const Vector3 &B);

	friend Vector3& operator += (Vector3 &A, const Vector3 &B);
	friend Vector3& operator -= (Vector3 &A, const Vector3 &B);
	friend Vector3& operator *= (Vector3 &A, const double &num);
	friend Vector3& operator /= (Vector3 &A, const double &num);
	friend Vector3& operator *= (Vector3 &A, const Vector3 &B);

	friend bool operator == (const Vector3 &A, const Vector3 &B);
	
	friend ostream& operator << (ostream& output, Vector3 &A);
    
    double& operator [] (int rank);

	double dot(const Vector3 &B);//µã³Ë
	double module();
	double module2();
	Vector3 unitVector();

	bool isZero();
	Vector3 reflect(const Vector3 &N);//N: unit normal Vector3
	bool refract(const Vector3 &N, const double n, Vector3& ref);//N; unit normal Vector3; n: relative index of refraction
};

#endif
