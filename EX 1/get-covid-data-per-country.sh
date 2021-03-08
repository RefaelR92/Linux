#!/bin/bash

mkdir countries
cd countries

countriesAPI='https://api.covid19api.com/countries'

wget ${countriesAPI}

tr '\{' '\n' <countries | awk -F : '{print $3}' >> countriesNames

rm countries
sed -i 's/"//g' countriesNames
sed -i 's/,ISO2//g' countriesNames
sed -i 's/united-states//g' countriesNames
sed -i '/^[[:space:]]*$/d' countriesNames

echo "countriesNames:"
cat countriesNames

# till here we have a list with all the countries.

apiCoutnryCall=https://api.covid19api.com/total/country/

while read country; do
	
	apiFromCountry="${apiCoutnryCall}${country}"
	wget "$apiFromCountry"
	sleep 2

    sed -i 's/,/\n/g' "${country}"
    sed -i '/^"Lat"/d' "${country}"
    sed -i '/^"Lon"/d' "${country}"
    sed -i '/^"City"/d' "${country}"
    sed -i '/^"CityCode"/d' "${country}"
    sed -i '/^"Province"/d' "${country}"
    sed -i '/^"CountryCode"/d' "${country}"
    sed -i '/^{"Country"/d' "${country}"
    sed -i 's/"//g' "${country}" 
    sed -i 's/{//g' "${country}"
    sed -i 's/}//g' "${country}"
    sed -i 's/]//g' "${country}"
    sed -i 's/T00:00:00Z//g' "${country}"
    sed -i '1d' "${country}"
    sed -i '/^ /d' "${country}"

		
 done <countriesNames
 
 find . -size 0 -delete
 cd ..
