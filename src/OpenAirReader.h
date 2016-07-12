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

#include <string>
#include <map>

class Airspace;

class OpenAirReader
{
public:
	OpenAirReader(std::multimap<int, Airspace>& outputMap);
	inline ~OpenAirReader() {}
	bool ReadFile(const std::string& fileName);

private:
	bool ParseAC(const std::string& line, Airspace& airspace);
	bool ParseAN(const std::string& line, Airspace& airspace);
	bool ParseAltitude(const std::string& line, const bool isTop, Airspace& airspace);
	bool ParseS (const std::string& line);
	bool ParseT (const std::string& line);
	bool ParseDP(const std::string& line, Airspace& airspace);
	bool ParseV(const std::string& line, Airspace& airspace);
	bool ParseDA(const std::string& line, Airspace& airspace);
	bool ParseDB(const std::string& line, Airspace& airspace);
	bool ParseDC(const std::string& line, Airspace& airspace);
	//bool ParseDY(const std::string& line, Airspace& airspace); // Airway not yet supported
	void ResetVar();
	bool InsertAirspace(Airspace& airspace);

	std::multimap<int, Airspace>* airspaces;
	bool varRotationClockwise;
	double varLat, varLon;
	double varWidth;
};
