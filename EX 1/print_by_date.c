#include "utils.h"
       
void printByDate(CountriesList* i_List, char* i_Key, char* i_Date);
int main(int argc, char* argv[])
{
	CountriesList* list = loadDataBase();
	printByDate(list, argv[4], argv[2]);
	deleteCountriesList(list);
	
	return 0;
}

void printByDate(CountriesList* i_List, char* i_Key, char* i_Date)
{
	int val;
	CountryList* pCountry = i_List->head;
	
	while(pCountry != NULL)
	{	
		val = getValByKeyAndDate(pCountry, i_Key, i_Date);
		printf("%s: %d\n", pCountry->name, val);
		pCountry = pCountry->next;
	}
}


