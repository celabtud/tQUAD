#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char ** argv)
{
	ifstream in,in2;
	ofstream out;
	string fname[100];
	string temp;
	int i=0,counter;
	unsigned long long int slice,bytes;

	if (argc==1)  in.open("flist.txt");
	else in.open(argv[1]);
	if (in.bad())
	{
		cerr<<"error in opening the filter list file!";
		return 1;
	}

	do
	{
		in>>fname[i];
		if (in.eof()) break;
		i++;
	}while(1);

	in.close();

	in2.open("MBWC_Matl.txt");
	out.open("MBWC_Matl_fil.txt");

	if (in2.bad() || out.bad())
	{
		cerr<<"error in opening the original flat profile and/or filtered output file!";
		return 1;
	}


	do
	{
		in2>>temp>>slice>>bytes;
		if (in2.eof()) break;

		for (counter=0;counter<i;counter++)
		{
			if( (fname[counter]+"(R)"==temp) || (fname[counter]+"(W)"==temp) )
			{
				out<<temp<<"\t"<<slice<<"\t"<<bytes<<endl;
				break;
			}
		}
	}while(1);

	in2.close();
	out.close();

	in.open("MBWC_Matl_Over.txt");
	out.open("MBWC_Matl_Over_fil.txt");

	if (in.bad() || out.bad())
	{
		cerr<<"error in opening the original overview flat profile and/or overview filtered output file!";
		return 1;
	}

	in>>temp;
	out<<temp<<endl;

	in>>temp;
	out<<temp<<endl;

	out<<i*2<<endl;

	for (counter=0;counter<i;counter++)
			out<<fname[counter]+"(R)"<<endl<<fname[counter]+"(W)"<<endl;

    in.close();
	out.close();

	cout<<"\n done!\n";
	return 0;
}
