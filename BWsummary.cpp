#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char ** argv)
{
	ifstream in;
	string dummy;
	long long int prv_s=-1;
	unsigned long long int totalTS,slice,bytes,ACTIVE_TS=0,max_bytes=0,max_ts_no=0,sum_bytes=0,inst_per_slice,
							interval_beg=0,interval_end=0;

	if (argc!=3) 
	{
	  cerr<<"\n Usage: BWsummary <total #TSs> <Inst. per TS>";
	  return 1;
	}
	
	in.open("MBWC_Matl_fil_spec.txt");
	if (in.bad())
	{
		cerr<<"error in opening the input data file!";
		return 1;
	}

	totalTS=atol(argv[1]);
	inst_per_slice=atol(argv[2]);
	
	if (inst_per_slice<1 || totalTS<1) 
	{
		cerr<<"error in time slice data!";
		return 1;
	}

	cout<<"\nThe function is active in the following time slices:\n";

	do
	{
		in>>dummy>>slice>>bytes;
		if (in.eof()) break;
		
		ACTIVE_TS++;
		sum_bytes+=bytes;		
		
		if (slice!=prv_s+1)
		{
			if(prv_s!=-1) { cout<<interval_beg<<"-"<<interval_end<<"\n"; }
			
			prv_s=slice;
			interval_beg=slice;
			interval_end=slice;
		}
		else {prv_s=slice; interval_end=slice;	}
				
		if (bytes>max_bytes) { max_bytes=bytes; max_ts_no=slice; }
	}while(1);

	in.close();
	
  	cout.precision(4);
  	cout.setf(ios::fixed,ios::floatfield);   // floatfield set to fixed	
	
    cout<<interval_beg<<"--"<<interval_end<<"\n"; 

    cout<<"The function was active in "<<ACTIVE_TS<<" time slices in total.\n"; 
    cout<<"That would count as "<<((long double)ACTIVE_TS/totalTS)*100<<" percent of the whole execution time.\n";

    cout<<"Average MBWU="<<sum_bytes/((long double)(inst_per_slice)*ACTIVE_TS)<<"\n"; 
    cout<<"Maximum MBWU occurs at time slice# "<<max_ts_no<<" and the value is "<<max_bytes/((long double)(inst_per_slice))<<"\n";
    
	return 0;
}
