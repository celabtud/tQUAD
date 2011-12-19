#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char ** argv)
{
	ifstream in,in2;
	ofstream out;
	string temp,fname[100];
	int i=0,fcounter;
	unsigned long long int slice,bytes,lslice,hslice,counter,* slices;

	if (argc!=4) 
	{
	  cerr<<"\n Usage: aggrview <functions list> <starting time slice> <ending time slice>";
	  return 1;
	}
	

	out.open("MBWC_Matl_fil_aggr(R).txt");
	if (out.bad())
	{
		cerr<<"error in opening the output data file!";
		return 1;
	}
	
	// read the lower and upper boundaries of time slice interval
	lslice=atol(argv[2]);
	hslice=atol(argv[3]);
	
	if (lslice<0 || hslice<0 || hslice<lslice) 
	{
		cerr<<"error in slice range!";
		return 1;
	}
	
	//allocate memory for the whole time slice array for one function
	slices=new unsigned long long int[hslice-lslice+1];

	in2.open(argv[1]);
	if (in2.bad())
	{
		cerr<<"error in opening the aggregate functions list file!";
		return 1;
	}

// read the names of the functions from the list file
	do
	{
		in2>>fname[i];
		if (in2.eof()) break;
		i++;
	}while(1);

	in2.close();


//for each function scan the whole flat profile once to extract all the information in the selected interval
for(fcounter=0;fcounter<i;fcounter++)
{			
	// go back to the beginning of the flat profile
	in.open("MBWC_Matl_fil.txt");
	if (in.bad())
	{
		cerr<<"error in opening the input data file!";
		return 1;
	}

	// initialize bytes accessed in all the time slices to zero
	for(counter=0;counter<hslice-lslice+1;counter++)
		slices[counter]=0;
	
	// read the whole flat ptofile
	do
	{
		in>>temp>>slice>>bytes;
		if (in.eof()) break;
		if( ((fname[fcounter]+"(R)")==temp) && (slice>=lslice) && (slice<=hslice) ) slices[slice-lslice]=bytes;
	}while(1);
	
	// write the data for the current function the aggregate output file
	for(counter=0;counter<hslice-lslice+1;counter++)
		out<<slices[counter]<<'\t';
	
	out<<'\n';

	in.close();	
}// end of for fcounter

	out.close();


	out.open("MBWC_Matl_fil_aggr(W).txt");
	if (out.bad())
	{
		cerr<<"error in opening the output data file!";
		return 1;
	}

//for each function scan the whole flat profile once to extract all the information in the selected interval
for(fcounter=0;fcounter<i;fcounter++)
{			

	// go back to the beginning of the flat profile
	in.open("MBWC_Matl_fil.txt");
	if (in.bad())
	{
		cerr<<"error in opening the input data file!";
		return 1;
	}

	// initialize bytes accessed in all the time slices to zero
	for(counter=0;counter<hslice-lslice+1;counter++)
		slices[counter]=0;
	
	// read the whole flat ptofile
	do
	{
		in>>temp>>slice>>bytes;
		if (in.eof()) break;
		if( ((fname[fcounter]+"(W)")==temp) && (slice>=lslice) && (slice<=hslice) ) slices[slice-lslice]=bytes;
	}while(1);
	
	// write the data for the current function the aggregate output file
	for(counter=0;counter<hslice-lslice+1;counter++)
		out<<slices[counter]<<'\t';
	
	out<<'\n';
	
	in.close();
}// end of for fcounter

	out.close();

	cout<<"\n done!\n";
	return 0;
}
