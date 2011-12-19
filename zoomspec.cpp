#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char ** argv)
{
	ifstream in;
	ofstream out;
	string temp,spec_func;
	int i=0,counter;
	unsigned long long int slice,bytes,lslice,hslice,max_bytes=0;

	if (argc!=4) 
	{
	  cerr<<"\n Usage: filspec <function name> <starting time slice> <ending time slice>";
	  return 1;
	}
	
	spec_func=argv[1];
	in.open("MBWC_Matl_fil.txt");
	if (in.bad())
	{
		cerr<<"error in opening the input data file!";
		return 1;
	}

	out.open("MBWC_Matl_fil_spec.txt");
	if (out.bad())
	{
		cerr<<"error in opening the output data file!";
		return 1;
	}

	
	lslice=atol(argv[2]);
	hslice=atol(argv[3]);
	
	if (lslice<0 || hslice<0 || hslice<lslice) 
	{
		cerr<<"error in slice range!";
		return 1;
	}

	do
	{
		in>>temp>>slice>>bytes;
		if (in.eof()) break;
		
		if( (spec_func==temp) && (slice>=lslice) && (slice<=hslice) )
			{
				out<<temp<<"\t"<<slice-lslice<<"\t"<<bytes<<endl;
				if (bytes>max_bytes) max_bytes=bytes;
			}
	}while(1);

	in.close();
	out.close();

	out.open("MBWC_Matl_Over_fil_spec.txt");

	if (out.bad())
	{
		cerr<<"error in opening the overview filtered output file!";
		return 1;
	}

	out<<hslice-lslice+1<<endl;
	out<<max_bytes<<endl;
	out<<1<<endl;
	out<<spec_func<<endl;
	
	out.close();

	cout<<"\n done!\n";
	return 0;
}
