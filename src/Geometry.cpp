//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#include "Geometry.h"
#include "OpenAir.h"
#include <cmath>
#include <cassert>

const int Geometry::LatLon::UNDEF_LAT = -91;
const int Geometry::LatLon::UNDEF_LON = -181;
const double Geometry::LatLon::SIXTYTH = 1.0 / 60;

const double Geometry::PI = 3.14159265358979323846;
const double Geometry::TWO_PI = 2 * PI;
const double Geometry::PI_2 = PI / 2;
const double Geometry::DEG2RAD = PI / 180;
const double Geometry::RAD2DEG = 180 / PI;
const double Geometry::NM2RAD = PI / (180 * 60);
const double Geometry::RAD2NM = (180 * 60) / PI;

double Geometry::resolution = 0.3 * NM2RAD; // 0.3 NM = 555.6 m

void Geometry::LatLon::convertDec2DegMin(const double& dec, int& deg, double& min) {
	double decimal = std::fabs(dec);
	deg=(int)floor(decimal);
	min = (decimal-deg)/SIXTYTH;
}

double Geometry::AbsAngle(const double& angle) { //to put angle in the range between 0 and 2PI
	assert(!isinf(angle));
	double absangle = std::fmod(angle, TWO_PI);
	if (absangle < 0) absangle += TWO_PI;
	assert(absangle >= 0 && absangle <= TWO_PI);
	return absangle;
}

// This requires pre-computation of distance
double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& d) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	if (lon1 == lon2) return lat1>lat2 ? PI : TWO_PI; //we are going to E or W
	return AbsAngle(sin(lon2 - lon1) < 0 ? acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))) : TWO_PI - acos((sin(lat2) - sin(lat1)*cos(d)) / (sin(d)*cos(lat1))));
}

// This not require pre-computation of distance
double Geometry::CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	if (lat2 == PI_2) return TWO_PI; //we are going to N pole
	if (lat2 == -PI_2) return PI; //we are going to S pole
	if (lon1 == lon2) return lat1>lat2 ? PI : TWO_PI; //we are going to E or W
	const double dlon = lon1 - lon2;
	const double coslat2 = cos(lat2);
	return AbsAngle(atan2(sin(dlon)*coslat2, cos(lat1)*sin(lat2) - sin(lat1)*coslat2*cos(dlon)));
}

double Geometry::CalcAngularDist(const double& lat1, const double& lon1, const double& lat2, const double& lon2) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(lat2 >= -PI_2 && lat2 <= PI_2);
	assert(lon2 >= -PI && lon2 <= PI);
	return 2.0 * asin(sqrt(pow(sin((lat1 - lat2) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((lon1 - lon2) / 2), 2)));
}

void Geometry::CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst, double& lat, double& lon) {
	assert(lat1 >= -PI_2 && lat1 <= PI_2);
	assert(lon1 >= -PI && lon1 <= PI);
	assert(dir > -PI && dir <= 2 * TWO_PI);
	const double sinlat1 = sin(lat1);
	const double cosdst = cos(dst);
	const double coslat1 = cos(lat1);
	const double sindst = sin(dst);
	lat = asin(sinlat1 * cosdst + coslat1 * sindst * cos(dir));
	lon = std::fmod(lon1 - atan2(sin(dir) * sindst * coslat1, cosdst - sinlat1 * sin(lat)) + PI, TWO_PI) - PI;
	assert(lat >= -PI_2 && lat <= PI_2);
	assert(lon >= -PI && lon <= PI);
}

Geometry::LatLon Geometry::CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst) {
	double lat=0, lon=0;
	CalcRadialPoint(lat1, lon1, dir, dst, lat, lon);
	return LatLon::CreateFromRadiants(lat,lon);
}

double Geometry::FindStep(const double& radius, const double& angle) {
	assert(angle >= 0 && angle <= TWO_PI);
	assert(radius >= 0);
	static const double smallRadius = 3 * NM2RAD; // 3 NM, radius under it the number of points will be decreased
	static const double m = 300 / smallRadius; // coeffcient to decrease points for small circles, 300 is default number of points for circles bigger than 3 NM
	int steps;
	if (radius < smallRadius) {
		steps = (int)((angle * (m * radius + 8)) / TWO_PI); // 8 is the minimum number of points for circles with radius -> 0
	}
	else steps = (int)((angle * radius) / resolution);
	return angle / steps;
}

double Geometry::DeltaAngle(const double angle, const double reference)
{
	assert(angle >= 0 && angle <= TWO_PI);
	assert(reference >= 0 && reference <= TWO_PI);
	double delta = angle - reference;
	if (delta > PI) delta -= TWO_PI;
	else if (delta < -PI) delta += TWO_PI;
	return delta;
}

bool Geometry::CalcBisector(const double& latA, const double& lonA, const double& latB, const double& lonB, const double& latC, const double& lonC, double& bisector) {
	const double courseBA = CalcGreatCircleCourse(latB, lonB, latA, lonA); //TODO: this can be done only one time
	const double courseBC = CalcGreatCircleCourse(latB, lonB, latC, lonC);

	double diff = DeltaAngle(courseBC, courseBA); //angle between the directions

	if (diff>0) { //positive difference: the internal angle in on the left
		diff = AbsAngle(diff);
		bisector = AbsAngle(courseBA + diff / 2);
		return false; // counter clockwise turn
	}
	else { //negative difference: the internal angle is on the right
		diff = AbsAngle(-diff);
		bisector = AbsAngle(courseBA - diff / 2);
		return true; // clockwise turn
	}
}

void Geometry::CalcSphericalTriangle(const double& a, const double& beta, const double& gamma, double& alpha, double& b, double& c)
{
	assert(a >= 0 && a <= TWO_PI);
	assert(beta >= 0 && gamma <= TWO_PI);
	assert(gamma >= 0 && gamma <= TWO_PI);
	alpha = acos(sin(beta)*sin(gamma)*cos(a) - cos(beta)*cos(gamma));
	b = atan2(sin(a)*sin(beta)*sin(gamma), cos(beta) + cos(gamma)*cos(alpha));
	c = atan2(sin(a)*sin(beta)*sin(gamma), cos(gamma) + cos(alpha)*cos(beta));
	assert(alpha >= 0 && alpha <= PI);
}


bool Geometry::CalcRadialIntersection(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& crs13, const double& crs23, double& lat3, double& lon3, double& dst13, double& dst23) {
	const double dst12 = CalcAngularDist(lat1, lon1, lat2, lon2); //TODO: this can be done only one time
	const double crs12 = CalcGreatCircleCourse(lat1, lon1, lat2, lon2, dst12); //TODO: this can be done only one time
	const double crs21 = CalcGreatCircleCourse(lat2, lon2, lat1, lon1, dst12); //TODO: this can be done only one time

	//double ang1 = std::fmod(crs13 - crs12 + PI, TWO_PI) - PI;
	//double ang2 = std::fmod(crs21 - crs23 + PI, TWO_PI) - PI;
	const double ang1 = AbsAngle(crs13 - crs12);
	const double ang2 = AbsAngle(crs21 - crs23);

	if ( (sin(ang1) == 0 && sin(ang2) == 0) || sin(ang1)*sin(ang2) < 0) return false;  //infinity of intersections or intersection ambiguous

	//ang1 = std::fabs(ang1);
	//ang2 = std::fabs(ang2);

	double ang3 = 0;
	CalcSphericalTriangle(dst12, ang1, ang2, ang3, dst13, dst23);

	assert(dst13 >= 0 && dst13 <= PI);
	assert(dst23 >= 0 && dst23 <= PI);

	CalcRadialPoint(lat1, lon1, crs13, dst13, lat3, lon3);

	return true;
}

bool Geometry::ArePointsOnArc(const LatLon& A, const LatLon& B, const LatLon& C, double& latc, double& lonc, double& radius, bool& clockwise) {
	const double latA = A.LatRad(); //TODO: avoid to recalculate always
	const double lonA = A.LonRad();
	const double latB = B.LatRad();
	const double lonB = B.LonRad();
	const double latC = C.LatRad();
	const double lonC = C.LonRad();

	const double dstAB = CalcAngularDist(latA, lonA, latB, lonB);
	const double crsAB = CalcGreatCircleCourse(latA, lonA, latB, lonB, dstAB);
	const double crsBA = CalcGreatCircleCourse(latB, lonB, latA, lonA, dstAB);

	const double dstBC = CalcAngularDist(latB, lonB, latC, lonC);
	const double crsBC = CalcGreatCircleCourse(latB, lonB, latC, lonC, dstBC);

	const double delta = DeltaAngle(crsBC, crsBA);
	if (delta == 0) return false; // aligned points  so not on an arc of circle!

	clockwise = delta < 0; // negative difference: internal angle is on the right so turning clockwise

	const double dstAB2 = dstAB / 2;
	double lat1=0, lon1=0;
	CalcRadialPoint(latA, lonA, crsAB, dstAB2, lat1, lon1);
	const double crs1A = CalcGreatCircleCourse(lat1, lon1, latA, lonA, dstAB2);
	const double crs1c = AbsAngle(clockwise ? crs1A - PI_2 : crs1A + PI_2);

	const double dstBC2 = dstBC / 2;
	double lat2 = 0, lon2 = 0;
	CalcRadialPoint(latB, lonB, crsBC, dstBC2, lat2, lon2);
	const double crs2B = CalcGreatCircleCourse(lat2, lon2, latB, lonB, dstBC2);
	const double crs2c = AbsAngle(clockwise ? crs2B - PI_2 : crs2B + PI_2);

	double radius2;
	CalcRadialIntersection(lat1, lon1, lat2, lon2, crs1c, crs2c, latc, lonc, radius, radius2);

	// Compare the two radius they must be similar
	const double diff = std::fabs(radius - radius2);
	const double limit = radius / 8;

	return diff < limit;
}

bool Point::Discretize(std::vector<LatLon>& output) const
{
	output.push_back(point); // Here it's easy :)
	return true;
}

void Point::WriteOpenAirGeometry(OpenAir* openAir) const
{
	openAir->WritePoint(this);
}

Sector::Sector(const LatLon& center, const double& radiusNM, const double& dir1, const double& dir2, const bool& isClockwise)
	: Geometry(center)
	, clockwise(isClockwise)
	, latc(center.LatRad())
	, lonc(center.LonRad())
	, A(CalcRadialPoint(latc, lonc, angleStart, radius))
	, B(CalcRadialPoint(latc, lonc, angleEnd, radius))
	, radius(radiusNM * NM2RAD)
	, angleStart(dir1 * DEG2RAD)
	, angleEnd(dir2 * DEG2RAD) {
}

Sector::Sector(const LatLon& center, const LatLon& pointStart, const LatLon& pointEnd, const bool& isClockwise)
	: Geometry(center)
	, clockwise(isClockwise)
	, latc(center.LatRad())
	, lonc(center.LonRad())
	, A(pointStart)
	, B(pointEnd) {
	const double lat1r = pointStart.LatRad();
	const double lon1r = -pointStart.LonRad();
	const double lat2r = pointEnd.LatRad();
	const double lon2r = -pointEnd.LonRad();
	radius = CalcAngularDist(latc, lonc, lat1r, lon1r);

	assert(radius > 0);
	assert(std::fabs((radius * RAD2NM) - (CalcAngularDist(latc, lonc, lat2r, lon2r) * RAD2NM)) < 0.2);

	angleStart = CalcGreatCircleCourse(latc, lonc, lat1r, lon1r);
	angleEnd = CalcGreatCircleCourse(latc, lonc, lat2r, lon2r);
}

bool Sector::Discretize(std::vector<LatLon>& output) const
{
	if (clockwise) {
		double e = angleStart <= angleEnd ? angleEnd : angleEnd + TWO_PI;
		const double step = FindStep(radius, AbsAngle(e - angleStart));
		assert(angleStart <= e);
		for (double a = angleStart; a < e; a += step)
			output.push_back(CalcRadialPoint(latc, lonc, a, radius));
	} else {
		const double s = angleStart >= angleEnd ? angleStart : angleStart + TWO_PI;
		const double step = FindStep(radius, AbsAngle(s - angleEnd));
		assert(s >= angleEnd);
		for (double a = s; a > angleEnd; a -= step)
			output.push_back(CalcRadialPoint(latc, lonc, a, radius));
	}
	return true;
}

void Sector::WriteOpenAirGeometry(OpenAir* openAir) const
{
	openAir->WriteSector(this);
}

Circle::Circle(const LatLon& center, const double& radiusNM)
	: Geometry(center)
	, radius(radiusNM * NM2RAD)
	, latc(center.LatRad())
	, lonc(center.LonRad()) {
}

bool Circle::Discretize(std::vector<LatLon>& output) const
{
	const double step = FindStep(radius, TWO_PI);
	for (double a = 0; a < TWO_PI; a += step) output.push_back(CalcRadialPoint(latc, lonc, a, radius));
	return true;
}

void Circle::WriteOpenAirGeometry(OpenAir* openAir) const
{
	openAir->WriteCircle(this);
}

/* Airway not supported yet
bool AirwayPoint::Discretize(std::vector<LatLon>& output) const
{
	// TODO...
	return false;
}
*/
