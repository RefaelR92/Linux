#include "utils.h"

void printGraph(CountryList* i_Country, char* i_Key);
int findMaxValue(CountryList* i_Country, char* i_Key);

int main(int argc, char* argv[])
{
	CountryList* Country = createCountryList(argv[2]);
	getCountryData(Country, argv[2]);
	printGraph(Country, argv[4]);
	deleteCountryList(Country);
	
	return 0;
}

void printGraph(CountryList* i_Country, char* i_Key)
{
	ListNode* pNode = i_Country->head;
	int max = findMaxValue(i_Country, i_Key)/NUM_OF_SPACES;
	
	while(pNode != NULL)
	{
		if(!strcmp(i_Key, "Confirmed"))
		{
			printf("%*c", pNode->confirmed/max, ' ');
			printf("%d\n", pNode->confirmed);
		}
		else if(!strcmp(i_Key, "Deaths"))
		{
			printf("%*c", pNode->deaths/max, ' ');
			printf("%d\n", pNode->deaths);
		}
		else if(!strcmp(i_Key, "Recovered"))
		{
			printf("%*c", pNode->recovered/max, ' ');
			printf("%d\n", pNode->recovered);
		}
		else if(!strcmp(i_Key, "Active"))
		{
			printf("%*c", pNode->active/max, ' ');
			printf("%d\n", pNode->active);
		}
		
		pNode = pNode->next;
	}
}

int findMaxValue(CountryList* i_Country, char* i_Key)
{
	int val = 0;
	ListNode* pNode = i_Country->head;
	
	while(pNode != NULL)
	{
		if(!strcmp(i_Key, "Confirmed"))
		{
			if(pNode->confirmed > val)
			{
				val = pNode->confirmed;
			}
		}
		else if(!strcmp(i_Key, "Deaths"))
		{
			if(pNode->deaths > val)
			{
				val = pNode->deaths;
			}
		}
		else if(!strcmp(i_Key, "Recovered"))
		{
			if(pNode->recovered > val)
			{
				val = pNode->recovered;
			}
		}
		else if(!strcmp(i_Key,"Active"))
		{
			if(pNode->active > val)
			{
				val = pNode->active;
			}
		}
		
		pNode = pNode->next;
	}
	
	return val;
}
