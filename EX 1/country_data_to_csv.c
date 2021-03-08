#include "utils.h"

void createMarksCsv(char* i_Filename, CountryList* i_CountryData);

int main(int argc, char* argv[])
{
	CountriesList* list = loadDataBase();	
	CountryList* Country = getCountryByName(list, argv[2]);
	createMarksCsv(argv[2], Country);
	deleteCountriesList(list);
		
	return 0;
}


void createMarksCsv(char* i_Filename, CountryList* i_CountryData)
{
	FILE* fp;
	char* countryToOpen = strcat(i_Filename,".csv");
	
	fp=fopen(getFilePath(countryToOpen),"w+");
	ListNode* currData = i_CountryData->head;
	fprintf(fp,"Date, Confirmed, Deaths, Recovered, Active");
	
	while(currData != NULL)
	{
		fprintf(fp,"\n%s ",currData->date);
		fprintf(fp,";%d ",currData->confirmed);	
		fprintf(fp,";%d ",currData->deaths);
		fprintf(fp,";%d ",currData->recovered);
		fprintf(fp,";%d ",currData->active);
		currData = currData->next;
	}
	
	fclose(fp);
}


