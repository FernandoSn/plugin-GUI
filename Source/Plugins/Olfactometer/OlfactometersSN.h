#pragma once

#include <string>

#define MAX_OLFACTOMETERS 5

//Serial numbers of the Arduino boards of each olfactometer.

//std::string PeteyA = "7553335303835121C142";
//std::string PeteyS = "95530343634351D08290";
//
//std::string ChanceA = "55732323930351414291";
//std::string ChanceS = "55735323935351709092";
//
//std::string ShadowA = "55737313231351600191";
//std::string ShadowS = "557363132383517102A0";
//
//std::string BruceA = "55735323335351906181";
//std::string BruceS = "55632313838351C09130";
//
//std::string BeastA = "75633313133351919050";
//std::string BeastS = "55732323930351414291";


std::string OlfacNames[MAX_OLFACTOMETERS] = { "Petey" ,
											"Chance" ,
											"Shadow" ,
											"Bruce" ,
											"Beast" };

std::string OlfacArd[MAX_OLFACTOMETERS] = { "7553335303835121C142" ,
											"55732323930351414291" ,
											"55737313231351600191" ,
											"55735323335351906181" ,
											"75633313133351919050" };

std::string OlfacSer[MAX_OLFACTOMETERS] = { "95530343634351D08290" ,
											"55735323935351709092" ,
											"557363132383517102A0" ,
											"55632313838351C09130" ,
											"55732323930351414291" };
