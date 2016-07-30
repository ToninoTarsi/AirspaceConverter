//============================================================================
// AirspaceConverter
// Since       : 14/6/2016
// Author      : Alberto Realis-Luc <alberto.realisluc@gmail.com>
// Web         : http://www.alus.it/AirspaceConverter
// Repository  : https://github.com/alus-it/AirspaceConverter.git
// Copyright   : (C) 2016 Alberto Realis-Luc
// License     : GNU GPL v3
//
// This source file is part of AirspaceConverter project
//============================================================================

#pragma once

#include <vector>

class Airspace;
class OpenAir;

class Geometry
{
friend class Airspace;
friend class OpenAir;

public:
	class LatLon
	{
	public:
		inline LatLon() : lat(UNDEF_LAT), lon(UNDEF_LON) {}
		inline LatLon(const double& latitude, const double& longitude) : lat(latitude), lon(longitude) {}
		inline static LatLon CreateFromRadiants(const double& latRad, const double& lonRad) { return LatLon(latRad * RAD2DEG, -lonRad * RAD2DEG); }
		inline double Lat() const { return lat; }
		inline double Lon() const { return lon; }
		inline double LatRad() const { return lat * DEG2RAD; }
		inline double LonRad() const { return -lon * DEG2RAD; }
		inline void GetLatLon(double& latitude, double& longitude) const { latitude = lat; longitude = lon; }
		inline void SetLatLon(const double& latitude, const double& longitude) { lat = latitude; lon = longitude; }
		inline void SetLatLonRad(const double latRad, const double lonRad) { lat = latRad * RAD2DEG; lon = -lonRad * RAD2DEG; }
		inline bool operator==(const LatLon& other) const { return other.lat == lat && other.lon == lon; }
		inline bool operator!=(const LatLon& other) const { return other.lat != lat || other.lon != lon; }
		inline void GetLatDegMin(int& deg, double& min) const { return convertDec2DegMin(lat, deg, min); }
		inline void GetLonDegMin(int& deg, double& min) const { return convertDec2DegMin(lon, deg, min); }
		inline char GetNorS() const { return lat > 0 ? 'N' : 'S'; }
		inline char GetEorW() const { return lon > 0 ? 'E' : 'W'; }

		static const int UNDEF_LAT, UNDEF_LON;

	private:
		double lat, lon;
		static void convertDec2DegMin(const double& dec, int& deg, double& min);
		static const double SIXTYTH;
	};

	virtual ~Geometry() {}
	virtual bool Discretize(std::vector<LatLon>& output) const = 0;
	static inline void SetResolution(double resolutionNM) { resolution = resolutionNM * NM2RAD; }
	inline const LatLon& GetCenterPoint() const { return point; }

protected:
	inline Geometry(const LatLon& center) : point(center) {}
	const LatLon point;
	static double resolution; // [rad] maximun distance between points when discretizing
	static const double TWO_PI;
	static const double DEG2RAD;
	static const double RAD2DEG;
	static const double NM2RAD;
	static const double RAD2NM;
	static double FindStep(const double& radius, const double& angle);
	static double DeltaAngle(const double angle, const double reference);
	static double AbsAngle(const double& angle);
	static double CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& d);
	static double CalcGreatCircleCourse(const double& lat1, const double& lon1, const double& lat2, const double& lon2);
	static double CalcAngularDist(const double& lat1, const double& lon1, const double& lat2, const double& lon2);
	static void CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst, double& lat, double& lon);
	static LatLon CalcRadialPoint(const double& lat1, const double& lon1, const double& dir, const double& dst);
	static bool CalcBisector(const double& latA, const double& lonA, const double& latB, const double& lonB, const double& latC, const double& lonC, double& bisector);
	static void CalcSphericalTriangle(const double& a, const double& beta, const double& gamma, double& alpha, double& b, double& c);
	static bool CalcRadialIntersection(const double& lat1, const double& lon1, const double& lat2, const double& lon2, const double& crs13, const double& crs23, double& lat3, double& lon3, double& dst13, double& dst23);
	static bool ArePointsOnArc(const LatLon& A, const LatLon& B, const LatLon& C, double& latc, double& lonc, double& radius, bool& clockwise);

private:
	static const double PI;
	static const double PI_2;
	virtual void WriteOpenAirGeometry(OpenAir* openAir) const = 0;
};

class Point : public Geometry
{
friend class OpenAir;
public:
	inline Point(const LatLon& latlon) : Geometry(latlon) {}
	inline Point(const double& lat, const double& lon) : Geometry(LatLon(lat,lon)) {}
	inline ~Point() {}
	bool Discretize(std::vector<LatLon>& output) const;

private:
	void WriteOpenAirGeometry(OpenAir* openAir) const;

};

class Sector : public Geometry
{
friend class OpenAir;
public:
	Sector(const LatLon& center, const double& radiusNM, const double& dir1, const double& dir2, const bool& isClockwise);
	Sector(const LatLon& center, const LatLon& pointStart, const LatLon& pointEnd, const bool& isClockwise);
	inline ~Sector() {}
	bool Discretize(std::vector<LatLon>& output) const;
	inline double GetRadiusNM() const { return RAD2NM * radius; }
	inline bool IsClockwise() const { return clockwise; }
	inline const LatLon& GetStartPoint() const { return A; }
	inline const LatLon& GetEndPoint() const { return B; }

private:
	void WriteOpenAirGeometry(OpenAir* openAir) const;

	const bool clockwise;
	const double latc, lonc; // [rad]
	LatLon A, B;
	double radius; // [rad]
	double angleStart, angleEnd; // [rad]


};

class Circle : public Geometry
{
friend class OpenAir;
public:
	Circle(const LatLon& center, const double& radiusNM);
	inline ~Circle() {}
	bool Discretize(std::vector<LatLon>& output) const;
	inline double GetRadiusNM() const { return RAD2NM * radius; }

private:
	const double radius; // [rad]
	const double latc, lonc; // [rad]

	void WriteOpenAirGeometry(OpenAir* openAir) const;
};

/* Airway for now not supported
class AirwayPoint : public Geometry
{
public:
	inline AirwayPoint(const double& lat, const double& lon, const double& widthNM) : Geometry(LatLon(lat, lon)), width(widthNM) {}
	inline ~AirwayPoint() {}
	bool Discretize(std::vector<LatLon>& output) const;

private:
	double width;
};

*/
