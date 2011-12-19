#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char ** argv)
{
	ifstream in,in2;
	
	string dummy;
	long double maxBW=0,curBW;
	unsigned long long int slice1,bytes1,slice2,bytes2,inst_per_ts,maxTS=0;

	if (argc!=2) 
	{
	  cerr<<"\n Usage: findmaxBW_TS <Inst. per TS>\n";
	  return 1;
	}
	
	in.open("dump1.txt");
	in2.open("dump2.txt");
	if (in.bad() || in2.bad())
	{
		cerr<<"error in opening the input data files!";
		return 1;
	}

	inst_per_ts=atol(argv[1]);
	
	if (inst_per_ts<1) 
	{
		cerr<<"error in time slice data!";
		return 1;
	}

	do
	{
		in>>dummy>>slice1>>bytes1;
		in2>>dummy>>slice2>>bytes2;
		
		if ( in.eof() )  
		  if (in2.eof()) break;
		  else {cout<<"\n oops!!! mismatch in the number of slices!\n"; return 1;}
		else if (in2.eof()) {cout<<"\n oops!!! mismatch in the number of slices!\n"; return 1;}
		
		if (slice1!=slice2) {cout<<"\n oops!!! mismatch in the slice number!!!\n"; return 1;}
		
		curBW=(bytes1+bytes2)/(long double)(inst_per_ts);
		if (curBW>maxBW) 
		{
			maxBW=curBW;
			maxTS=slice1;
		}
		
	}while(1);

	in.close();
	in2.close();
	
  	cout.precision(4);
  	cout.setf(ios::fixed,ios::floatfield);   // floatfield set to fixed	
	
    cout<<"\nMaximum Combined(R+W) MBWU occurs at time slice# "<<maxTS<<" and the value is "<<maxBW<<"\n";
    
	return 0;
}
