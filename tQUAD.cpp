// final revision, May 10th, 2010 by Arash Ostadzadeh
// temporal Quantitative Usage Analysis of Data (tQUAD) v0.2
// first attempt to provide the option to exclude/include the Stack memory region accesses from the total bandwidth analysis

#include "pin.H"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <stack>
#include <map>


#ifdef WIN32
#define DELIMITER_CHAR '\\'
#else 
#define DELIMITER_CHAR '/'
#endif



/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

   char main_image_name[40];
   ofstream dumpf;			// the dump file for memory slices snapshots, later converted to a complete report file
   ofstream reportf;
   ofstream matlabf;

   UINT64 Memory_Snapshot_Interval;   // The interval snapshot based on the number of instructions

   UINT64 CurrentInstructions=0;			// current number of executed instructions, interpreted as the time frame for taking snapshot
   UINT64 MaxBW=0;
   
   typedef struct
   {
   	   UINT64 Readbytes;
   	   UINT64 Writebytes;
   } MACC; 

   stack <string> CallStack;   // just to keep track of the function names we care for (our call stack instead of the real stack)

   map <string,MACC *> FnametoList;   // This will be the list of functions which have demonstrated memory accesses for the current interval snapshot. The list will be reset at the beginning of each recording process

   // these variables are used to speedup the lookup process, most likely the new memory access instruction is issued by the current function in charge, so no need to go thru the entire map to find it, if this is not the case we will conduct a search on the map
   string current_f_name;
   MACC *current_f_MACC_data;

   //slice number
   UINT64 TimeSlice=1;
   
   BOOL Uncommon_Functions_Filter=TRUE;
   BOOL No_Stack_Flag = FALSE;   // a flag showing our interest to include or exclude stack memory accesses in analysis. The default value indicates tracing also the stack accesses. Can be modified by 'ignore_stack_access' command line switch
   

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<UINT64> KnobSlice(KNOB_MODE_WRITEONCE, "pintool",
    "slice","500000", "Specify time slice interval for taking snapshots in terms of the number of instructions");

KNOB<BOOL> KnobIgnoreUncommonFNames(KNOB_MODE_WRITEONCE, "pintool",
    "filter_uncommon_functions","1", "Filter out uncommon function names which are unlikely to be defined by user (beginning with question mark, underscore(s), etc.)");

KNOB<BOOL> KnobIgnoreStackAccess(KNOB_MODE_WRITEONCE, "pintool",
    "ignore_stack_access","0", "Ignore memory accesses within application's stack region");



/* ===================================================================== */

void AugmentDump()    
{
	map <string,MACC*> :: const_iterator pIter;

	cout<<(char)(13)<<"Writing snapshot data for time slice #"<<TimeSlice;

	dumpf<<"Slice # "<<TimeSlice<<endl;
	for ( pIter = FnametoList.begin( ) ; pIter != FnametoList.end( ) ; pIter++ )
     {
     	if (pIter->second->Readbytes)
		{
		  dumpf << (pIter -> first)+"(R)" <<endl;
		  dumpf << pIter->second->Readbytes <<endl;
		  matlabf << (pIter -> first)+"(R)"<<'\t'<<TimeSlice<<'\t'<<pIter->second->Readbytes <<endl;
		  if (pIter->second->Readbytes>MaxBW) MaxBW=pIter->second->Readbytes;
		  pIter->second->Readbytes=0;
		}
		if (pIter->second->Writebytes)
		{
     	  dumpf << (pIter -> first)+"(W)" <<endl;
		  dumpf << pIter->second->Writebytes <<endl;
		  matlabf << (pIter -> first)+"(W)"<<'\t'<<TimeSlice<<'\t'<<pIter->second->Writebytes <<endl;
		  if (pIter->second->Writebytes>MaxBW) MaxBW=pIter->second->Writebytes;
		  pIter->second->Writebytes=0;
		}
	 }
     
	dumpf << "---------------------------------------------------" <<endl;
}

void CreateGenReport()
{
	
   map <string,MACC*> :: const_iterator pIter;
   // map <string,MACC*> :: size_type size;
	
   reportf.open("MBWCover.txt");

   if(reportf.bad()) 
	{
		cerr<<"\nCan't create the general snapshot overview file..."<<endl;
		return;
	}

   reportf<<"Total time slices = "<<TimeSlice<<endl;
   reportf<<"Maximum BW consumption in a time slice = "<<MaxBW<<endl;
   reportf<<"Total number of function graphs = "<<FnametoList.size()*2<<endl;


   for ( pIter = FnametoList.begin( ) ; pIter != FnametoList.end( ) ; pIter++ )
   {	  
	   reportf << (pIter -> first)+"(R)" <<endl;
	   reportf << (pIter -> first)+"(W)" <<endl;
   }
   
   reportf.close();
   //*******************

   reportf.open("MBWC_Matl_Over.txt");

   if(reportf.bad()) 
	{
		cerr<<"\nCan't create the general snapshot overview file..."<<endl;
		return;
	}

   reportf<<TimeSlice<<endl;
   reportf<<MaxBW<<endl;
   reportf<<FnametoList.size()*2<<endl;

   for ( pIter = FnametoList.begin( ) ; pIter != FnametoList.end( ) ; pIter++ )
   {	  
	   reportf << (pIter -> first)+"(R)" <<endl;
	   reportf << (pIter -> first)+"(W)" <<endl;
   }
   
   reportf.close();

}
//------------------------------------------------------------------

/* This is the old routine........................................

VOID EnterFC(char *name,bool flag) 
{

    string RName(name);

	if(
	    RName=="GetPdbDll" || 
	    RName=="DebuggerRuntime" || 
	    RName=="atexit" || 
	    RName=="failwithmessage" ||
		RName=="pre_c_init" ||
		RName=="pre_cpp_init" ||
		RName=="mainCRTStartup" ||
		RName=="NtCurrentTeb" ||
		RName=="check_managed_app" ||
		RName=="DebuggerKnownHandle" ||
		RName=="DebuggerProbe" ||
		RName=="failwithmessage" ||
		RName=="unnamedImageEntryPoint"
	   
	   ) return;
		 
    if( flag && name[0]!='?' )  // found in the main image, so do update the current routine in our call stack
   				CallStack.push(RName);
}

end of the old routine .........................................*/

// the new EnterFC routine!

VOID EnterFC(char *name,bool flag) 
{

  // revise the following in case you want to exclude some unwanted functions under Windows and/or Linux

  if (!flag) return;   // not found in the main image, so skip the current function name update

#ifdef WIN32

  if (Uncommon_Functions_Filter)

	if(		
		name[0]=='_' ||
		name[0]=='?' ||
		!strcmp(name,"GetPdbDll") || 
	    	!strcmp(name,"DebuggerRuntime") || 
	    	!strcmp(name,"atexit") || 
	    	!strcmp(name,"failwithmessage") ||
		!strcmp(name,"pre_c_init") ||
		!strcmp(name,"pre_cpp_init") ||
		!strcmp(name,"mainCRTStartup") ||
		!strcmp(name,"NtCurrentTeb") ||
		!strcmp(name,"check_managed_app") ||
		!strcmp(name,"DebuggerKnownHandle") ||
		!strcmp(name,"DebuggerProbe") ||
		!strcmp(name,"failwithmessage") ||
		!strcmp(name,"unnamedImageEntryPoint")
	   ) return;
#else
  if (Uncommon_Functions_Filter)

	if( name[0]=='_' || name[0]=='?' || 
            !strcmp(name,"call_gmon_start") || !strcmp(name,"frame_dummy") 
          ) return;
#endif
    

	// update the current function name	 
	string RName(name);
	CallStack.push(RName);
}

/* ===================================================================== */

INT32 Usage()
{
    cerr <<
        "\nThis tool creates a detailed report of memory accesses bandwidth consumption of all functions in an application within predefined interval snapshots.\n";

    
    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}


/* ===================================================================== */


VOID  Return(VOID *ip)   // monitoring the return instruction just lets us to update our call stack if necessary!
{
       string fn_name = RTN_FindNameByAddress((ADDRINT)ip);


       if( (CallStack.top()==fn_name) && (!CallStack.empty()) )
	    // if I like the name of the function which is returning control!

						 CallStack.pop();
			
}

/* ===================================================================== */

VOID UpdateCurrentFunctionName(RTN rtn,VOID *v)
{
	  
		bool flag;
		char *s=new char[80];
		string RName;


		flag=(!((IMG_Name(SEC_Img(RTN_Sec(rtn))).find(main_image_name)) == string::npos));
			
		RName=RTN_Name(rtn);

		strcpy(s,RName.c_str());


		RTN_Open(rtn);
            
        // Insert a call at the entry point of a routine to push the current routine to CallStack
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)EnterFC, IARG_PTR, s, IARG_BOOL, flag, IARG_END);

        // Insert a call at the exit point of a routine to pop the current routine from CallStack if we have the routine on the top
	    // RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)exitFc, IARG_PTR, RName.c_str(), IARG_END);
        
	    RTN_Close(rtn);
	
}
/* ===================================================================== */

VOID Fini(INT32 code, VOID *v) // to be refined ......................................
{
    
	if (CurrentInstructions) AugmentDump();  // put the last slice's data in the dump file!
	else TimeSlice--;
	dumpf.close();
	matlabf.close();

	cout << "\nFinished executing the instrumented application..." << endl;
	cout << "\nCreating the report file..." ;

	CreateGenReport();


    //MakeReportFile();
    cout << "done!" << endl <<endl;
}

// increment routine for the total instruction counter
VOID IncreaseTotalInstCounter()
{
	CurrentInstructions++;
}

// checks whether or not it's the time for augmenting the report file!
VOID Slice_checkpoint()
{
   
	if (CurrentInstructions==Memory_Snapshot_Interval) 
	 {
	   AugmentDump();
       TimeSlice++;
	   CurrentInstructions=0;
	 }

}

VOID IncreaseRead(INT32 size, BOOL isPrefetch)
{
	if(!isPrefetch) 
	 {
		 string temp=CallStack.top();
		 if(temp == current_f_name)   // we are still in the territory of the current function
			 current_f_MACC_data->Readbytes=current_f_MACC_data->Readbytes+size;

		 else
		 {
           if(FnametoList.find(temp)==FnametoList.end())  // this is the first time I see this function name, so no entry in MAP has been created this function yet
		   {
             current_f_name=temp;
			 current_f_MACC_data=new MACC;
			 if(!current_f_MACC_data)
			 {
				cerr<<"\nMemory allocation failure..."<<endl;
				return;
			 }
		     current_f_MACC_data->Readbytes=size;
			 current_f_MACC_data->Writebytes=0;
			 FnametoList[current_f_name]=current_f_MACC_data;  // put the new record in the map
		   }
		   else  // this function has been previously active somewhere, so the record exists in the map, just update the bytes, and the current function name!
		   {
		     current_f_MACC_data=FnametoList[temp];
			 current_f_MACC_data->Readbytes=current_f_MACC_data->Readbytes+size;
             current_f_name=temp;
		   }
		 } // end of external else
	}// end of if this is not a prefetch!
}


// the version used when we also want to check the stack pointer 
VOID IncreaseReadSP(VOID *ESP, VOID *addr, INT32 size, BOOL isPrefetch)
{
	if(!isPrefetch) 
	 {
	     if (addr >= ESP) return;  // if we are reading from the stack range, ignore this access
	 	 string temp=CallStack.top();
		 if(temp == current_f_name)   // we are still in the territory of the current function
			 current_f_MACC_data->Readbytes=current_f_MACC_data->Readbytes+size;

		 else
		 {
           if(FnametoList.find(temp)==FnametoList.end())  // this is the first time I see this function name, so no entry in MAP has been created this function yet
		   {
             current_f_name=temp;
			 current_f_MACC_data=new MACC;
			 if(!current_f_MACC_data)
			 {
				cerr<<"\nMemory allocation failure..."<<endl;
				return;
			 }
		     current_f_MACC_data->Readbytes=size;
			 current_f_MACC_data->Writebytes=0;
			 FnametoList[current_f_name]=current_f_MACC_data;  // put the new record in the map
		   }
		   else  // this function has been previously active somewhere, so the record exists in the map, just update the bytes, and the current function name!
		   {
		     current_f_MACC_data=FnametoList[temp];
			 current_f_MACC_data->Readbytes=current_f_MACC_data->Readbytes+size;
             current_f_name=temp;
		   }
		 } // end of external else
	}// end of if this is not a prefetch!
}


VOID IncreaseWrite(INT32 size, BOOL isPrefetch)
{
	if(!isPrefetch) 
	 {
		 string temp=CallStack.top();
		 if(temp == current_f_name)   // we are still in the territory of the current function
			 current_f_MACC_data->Writebytes=current_f_MACC_data->Writebytes+size;

		 else
		 {
           if(FnametoList.find(temp)==FnametoList.end())  // this is the first time I see this function name, so no entry in MAP has been created this function yet
		   {
             current_f_name=temp;
			 current_f_MACC_data=new MACC;
			 if(!current_f_MACC_data)
			 {
				cerr<<"\nMemory allocation failure..."<<endl;
				return;
			 }
		     current_f_MACC_data->Readbytes=0;
			 current_f_MACC_data->Writebytes=size;
			 FnametoList[current_f_name]=current_f_MACC_data;  // put the new record in the map
		   }
		   else  // this function has been previously active somewhere, so the record exists in the map, just update the bytes, and the current function name!
		   {
		     current_f_MACC_data=FnametoList[temp];
			 current_f_MACC_data->Writebytes=current_f_MACC_data->Writebytes+size;
             current_f_name=temp;
		   }
		 } // end of external else
	}// end of if this is not a prefetch!
}

// the version used when we also want to check the stack pointer 

VOID IncreaseWriteSP(VOID *ESP, VOID *addr, INT32 size, BOOL isPrefetch)
{
	if(!isPrefetch) 
	 {
	     if (addr >= ESP) return;  // if we are reading from the stack range, ignore this access
		 string temp=CallStack.top();
		 if(temp == current_f_name)   // we are still in the territory of the current function
			 current_f_MACC_data->Writebytes=current_f_MACC_data->Writebytes+size;

		 else
		 {
           if(FnametoList.find(temp)==FnametoList.end())  // this is the first time I see this function name, so no entry in MAP has been created this function yet
		   {
             current_f_name=temp;
			 current_f_MACC_data=new MACC;
			 if(!current_f_MACC_data)
			 {
				cerr<<"\nMemory allocation failure..."<<endl;
				return;
			 }
		     current_f_MACC_data->Readbytes=0;
			 current_f_MACC_data->Writebytes=size;
			 FnametoList[current_f_name]=current_f_MACC_data;  // put the new record in the map
		   }
		   else  // this function has been previously active somewhere, so the record exists in the map, just update the bytes, and the current function name!
		   {
		     current_f_MACC_data=FnametoList[temp];
			 current_f_MACC_data->Writebytes=current_f_MACC_data->Writebytes+size;
             current_f_name=temp;
		   }
		 } // end of external else
	}// end of if this is not a prefetch!
}


// Is called for every instruction and instruments returns, reads and writes
VOID Instruction(INS ins, VOID *v)
{

	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncreaseTotalInstCounter, IARG_END);


	
     if (INS_IsRet(ins))  // we are monitoring the 'ret' instructions since we need to know when we are leaving functions in order to update our own virtual 'Call Stack'. The mechanism to inject instrumentation code to update the Call Stack (pop) upon leave is not implemented directly contrary to the dive in mechanism. Could be a point for further improvement?! ...
     {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)Return, IARG_INST_PTR, IARG_END);
     }

 if (!No_Stack_Flag)  // Stack accesses should be counted and they are ok!!!
  {
     if (INS_IsMemoryRead(ins) || INS_IsStackRead(ins) )
     {

            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)IncreaseRead,
                IARG_MEMORYREAD_SIZE,
                IARG_UINT32, INS_IsPrefetch(ins),
                IARG_END);
     }

     if (INS_HasMemoryRead2(ins))
     {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)IncreaseRead,
                IARG_MEMORYREAD_SIZE,
                IARG_UINT32, INS_IsPrefetch(ins),
                IARG_END);
     }

     if (INS_IsMemoryWrite(ins) || INS_IsStackWrite(ins) ) 
     {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)IncreaseWrite,
                IARG_MEMORYWRITE_SIZE,
                IARG_UINT32, INS_IsPrefetch(ins),
                IARG_END);
      }
    } // end of Stack is ok!

 else  // ignore stack access
  {
     if (INS_IsMemoryRead(ins))
     {

            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)IncreaseReadSP,
				IARG_REG_VALUE, REG_STACK_PTR,
                IARG_MEMORYREAD_EA,
                IARG_MEMORYREAD_SIZE,
                IARG_UINT32, INS_IsPrefetch(ins),
                IARG_END);
     }

     if (INS_HasMemoryRead2(ins))
     {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)IncreaseReadSP,
				IARG_REG_VALUE, REG_STACK_PTR,
                IARG_MEMORYREAD2_EA,
                IARG_MEMORYREAD_SIZE,
                IARG_UINT32, INS_IsPrefetch(ins),
                IARG_END);
     }

     if (INS_IsMemoryWrite(ins) ) 
     {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)IncreaseWriteSP,
				IARG_REG_VALUE, REG_STACK_PTR,
                IARG_MEMORYWRITE_EA,
                IARG_MEMORYWRITE_SIZE,
                IARG_UINT32, INS_IsPrefetch(ins),
                IARG_END);
      }
  
  } // end of ignore stack 


	// check for the end of the current time slice to put the data in the report file
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Slice_checkpoint, IARG_END);

}
/////////////////////////////////////////////////////////////////////////
const char * StripPath(const char * path)
{
    const char * file = strrchr(path,DELIMITER_CHAR);
    if (file)
        return file+1;
    else
        return path;
}
/* ===================================================================== */

int  main(int argc, char *argv[])
{
   char temp[100];
    
   dumpf.open("MBWC.txt");
   matlabf.open("MBWC_Matl.txt");

   if(dumpf.bad() || matlabf.bad()) 
	{
		cerr<<"\nCan't create the dump file..."<<endl;
		return 1;
	}
	
   // assume 'Out_of_the_main_function_scope' as the first current routine
   current_f_name="Out_of_the_main_function_scope";

   current_f_MACC_data=new MACC;

   if(!current_f_MACC_data)
	{
		cerr<<"\nMemory allocation failure..."<<endl;
		return 1;
	}

   current_f_MACC_data->Readbytes=0;
   current_f_MACC_data->Writebytes=0;

   FnametoList[current_f_name]=current_f_MACC_data;  // put the first record in the map
   CallStack.push("Out_of_the_main_function_scope"); // put the first record in our callstack

   // parse the command line arguments to get the main image name
    for (int i=1;i<argc-1;i++)
	  if (!strcmp(argv[i],"--"))
	    strcpy(temp,argv[i+1]);

    strcpy(main_image_name,StripPath(temp));
	
	cout<<endl;
    PIN_InitSymbols();

    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

//	Memory_Snapshot_Interval=atoi(KnobSlice.Value().c_str());
	Memory_Snapshot_Interval=KnobSlice.Value();
    Uncommon_Functions_Filter=KnobIgnoreUncommonFNames.Value(); // interested in uncommon function names or not?
    No_Stack_Flag=KnobIgnoreStackAccess.Value(); // Stack access ok or not?


//	if (errno == ERANGE)
//    {
//       cerr<<"\nOverflow condition occurred in command line conversion of the time slice.\n";
//	   return 2;
//    }
	if (Memory_Snapshot_Interval<1) Memory_Snapshot_Interval=1;


	if (Memory_Snapshot_Interval<1000)
		cout<<"\nWarning: time slice set to a very small value which could result in a huge report file!\n";

    RTN_AddInstrumentFunction(UpdateCurrentFunctionName,0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    cout << "Starting the application to be analysed..." << endl;
    PIN_StartProgram();
    
    return 0;
}
