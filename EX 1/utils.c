#include "utils.h"


int x=4;
CountriesList* loadDataBase()
{

 	char* fileToOpen = getFilePath("countriesNames");
	FILE* countriesFptr;
	countriesFptr = fopen(fileToOpen, "r");
	char country[100];
	CountriesList* Countries = createCountriesList();
	
	while(fscanf(countriesFptr,"%s", country) != EOF)
	{
		CountryList* Country = createCountryList(country);
		getCountryData(Country, country);
		addCountryList(Countries, Country);
	}

	fclose(countriesFptr);
	return Countries;
}

void getCountryData(CountryList* i_CountryList, char* i_CountryName)
{
	FILE* fkptr;
	char* date;
	int confirmed, deaths, recovered, active;
	int counter = 0; 
	char* tempInfo;
 	char* fileToOpen = getFilePath(i_CountryName);	

	if(fkptr = fopen(fileToOpen, "r"))
	{
		char stringToRead[100];
		char* infoToPrint = (char*)malloc(100);
		
		while(fscanf(fkptr,"%s", stringToRead) != EOF)
		{
			counter++;
			
			if(strstr(stringToRead, "Confirmed"))
			{
				tempInfo = strstr(stringToRead, "Confirmed");
				confirmed = atoi(tempInfo+10);
			}
			else if(strstr(stringToRead, "Deaths"))
			{
				tempInfo = strstr(stringToRead, "Deaths");
				deaths = atoi(tempInfo+7);
			}
			else if(strstr(stringToRead, "Recovered"))
			{
				tempInfo = strstr(stringToRead, "Recovered");
				recovered = atoi(tempInfo+10);
			}
			else if(strstr(stringToRead, "Active"))
			{
				tempInfo = strstr(stringToRead, "Active");
				active = atoi(tempInfo+7);
			}
			else if(strstr(stringToRead, "Date"))
			{
				tempInfo = strstr(stringToRead, "Date");
				date = strdup(tempInfo+5);
			}
		
			if (counter % NUMBER_OF_PARAMETERS_IN_NODE == 0)
			{
				ListNode* newNode = createListNode(date, confirmed, deaths, recovered, active);
				addListNode(i_CountryList, newNode);
			}
		}
		
		fclose(fkptr);
	}
}


CountryList* getCountryByName(CountriesList* i_CountriesList, char* i_CountryName)
{
	CountryList* pCountry = i_CountriesList->head;
	bool keepRunning = TRUE;
	
	while(pCountry != NULL && keepRunning) 
	{
		if(!strcmp(pCountry->name,i_CountryName))
		{
			keepRunning = FALSE;
		}
		else
		{
			pCountry = pCountry->next;
		}
	}
	
	return pCountry;
}

int getValByKeyAndDate(CountryList* i_Country, char* i_Key, char* i_Date)
{	
	int res;
	ListNode* pNode = i_Country->head;
	bool keepRunning = TRUE;
	
	while(pNode != NULL && keepRunning)
	{
		if(!strcmp(pNode->date,i_Date))
		{
			if(!strcmp(i_Key,"Confirmed"))
			{
				res = pNode->confirmed;
				keepRunning = FALSE;
			}
			else if(!strcmp(i_Key,"Deaths"))
			{
				res = pNode->deaths;
				keepRunning = FALSE;
			}
			else if(!strcmp(i_Key,"Recovered"))
			{
				res = pNode->recovered;
				keepRunning = FALSE;
			}
			else if(!strcmp(i_Key,"Active"))
			{
				res = pNode->active;
				keepRunning = FALSE;
			}
		}

		pNode = pNode->next;
	}
	
	return res;
}

CountriesList* createCountriesList()
{
	CountriesList* list = (CountriesList*)malloc(sizeof(CountriesList));
	list->head = NULL;
	list->tail = NULL;
	return list;
}

CountryList* createCountryList(char* i_Country)
{
	CountryList* list = (CountryList*)malloc(sizeof(CountryList));
	list->name = strdup(i_Country);
	list->head = NULL;
	list->tail = NULL;
	list->next = NULL;
	
	return list;
}

ListNode* createListNode(char* i_Date, int i_Confirmed, int i_Deaths, int i_Recovered, int i_Active)
{
	ListNode* node = (ListNode*)malloc(sizeof(ListNode));
	node->date = strdup(i_Date);
	node->confirmed = i_Confirmed;
	node->deaths = i_Deaths;
	node->recovered = i_Recovered;
	node->active = i_Active;
	node->next = NULL;
	
	return node;
}

void addCountryList(CountriesList* i_List, CountryList* i_Node)
{
	if(i_List->head == NULL)
	{
		i_List->head = i_Node;
		i_List->tail = i_Node;
	}
	else
	{
		i_List->tail->next = i_Node;
		i_List->tail = i_Node;
	}
}

void addListNode(CountryList* i_List, ListNode* i_Node)
{	
	if(i_List->head == NULL)
	{
		i_List->head = i_Node;
		i_List->tail = i_Node;
	}
	else
	{
		i_List->tail->next = i_Node;
		i_List->tail = i_Node;
	}
}

void deleteListNode(ListNode* i_Node)
{
	free(i_Node->date);
	free(i_Node);
}

void deleteCountryList(CountryList* i_List)
{
	ListNode* nextNode = i_List->head;
	ListNode* curr = nextNode;
	
	while(nextNode != NULL)
	{
		curr = nextNode;
		nextNode = nextNode->next;
		deleteListNode(curr);
	}
	
	free(i_List->name);
	free(i_List);
}

void deleteCountriesList(CountriesList* i_List)
{
	CountryList* nextList = i_List->head;
	CountryList* curr = nextList;
	
	while(nextList != NULL)
	{
		curr = nextList;
		nextList = nextList->next;
		deleteCountryList(curr);
	}
	

	free(i_List);
}

char* getFilePath(char* i_CountryName)
{
 	char cwd[PATH_MAX];
 	getcwd(cwd, sizeof(cwd));
 	char* fileToOpen = strcat(cwd,"/countries/");
 	
 	return strcat(fileToOpen, i_CountryName);
}
