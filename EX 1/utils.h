#pragma once 
#ifndef __UTILES_H__
#define __UTILES_H__
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#define bool int
#define TRUE 1
#define FALSE 0
#define NUMBER_OF_PARAMETERS_IN_NODE 5
#define NUM_OF_SPACES 100

typedef struct listNode
{
	char* date;
	int confirmed;
	int deaths;
	int recovered;
	int active;
	struct listNode* next;
}ListNode;

typedef struct countryList
{	
	char* name;
	ListNode* head;
	ListNode* tail;
	struct countryList* next;
}CountryList;


typedef struct countriesList
{
	CountryList* head;
	CountryList* tail;
}CountriesList;

CountriesList* loadDataBase();
void getCountryData(CountryList* i_CountryList, char* i_CountryName);
CountryList* getCountryByName(CountriesList* i_CountriesList, char* i_CountryName);
int getValByKeyAndDate(CountryList* i_Country, char* i_Key, char* i_Date);
CountriesList* createCountriesList();
CountryList* createCountryList(char* i_Country);
ListNode* createListNode(char* i_Date, int i_Confirmed, int i_Deaths, int i_Recovered, int i_Active);
void addCountryList(CountriesList* i_List, CountryList* i_Node);
void addListNode(CountryList* i_List, ListNode* i_Node);
void deleteListNode(ListNode* i_Node);
void deleteCountryList(CountryList* i_List);
void deleteCountriesList(CountriesList* i_List);
char* getFilePath(char* i_CountryName);

#endif
