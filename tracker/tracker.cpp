#include <bits/stdc++.h>
#include <pthread.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <openssl/sha.h>
#include <fstream>


#define SA struct sockaddr

using namespace std;


unordered_map<string, string> pieceWise;
unordered_map<string, string> login;
unordered_map<string, bool> checkLogin;
uint16_t tracker_port;
unordered_map<string, string> adminGroups;
unordered_map<string, set < string>> gMembers;
string tracker_ip, uploadFile;
vector<string> groupList;
unordered_map<string, set < string>> gRequests;
unordered_map<string, string> curFilesize;
unordered_map<string, string> convertPort;

FILE *fpointer;

 void appendLog(string str)
{
    fprintf(fpointer, "%s", str.c_str());
}


bool pathExist(const string &str)
{
	struct stat itr;
	bool ret;
	if (stat(str.c_str(), &itr) == 0)
	{
		ret= true;
	}
	else
	{
		ret= false;
	}
	return ret;
}

vector<string> seperateString(string str, string f)
{
	
	size_t p = 0;
	vector<string> res;
	 
	p = str.find(f);
	string t;
	while (p != string::npos)
	{
		 
		t = str.substr(0, p);
		res.push_back(t);
		 
		str.erase(0, p + f.length());

		 
	
	p = str.find(f);
	
	}

	res.push_back(str);
	 
	return res;
}
unordered_map<string, unordered_map<string, set< string>>> uploadList;
pair<int,string> getIPAndPortFromFile(char* fn){
	string name ;
	name= string(fn);
    string l;
	l="";
    ifstream input_file(name);

    if (input_file.is_open()) {
		getline(input_file, l);
		vector<string> tip;
        tip	= seperateString(l,":");
	 
		string ip;
		ip=tip[0];
		int port;
		port= stoi(tip[1]);
		 
        pair<int,string> ret=make_pair(port,ip);
		return ret;
    }

	else{
		
		 
		cout<<"Cannot open the file"<<endl;
		appendLog("Cannot open the file\n");
		pair<int,string> ret=make_pair(0,"0.0.0.0");
		return ret;
	}
}

void *closeServer(void *arg)
{
string q;
	while (1)
	{
		
		getline(cin, q);
		string str="quit";
		if (q == str)
		{
			appendLog("Exit\n");


	appendLog("Tracker logs finished\n\n");

fclose(fpointer);
			exit(0);
		}
	}
}



int validateLogin(string uid, string p)
{


if ((login.find(uid) == login.end())||login[uid] != p){
	int ret=-1;
		return ret;
		}

	
	if (checkLogin.find(uid) == checkLogin.end())
	{
		int ret=0;
		checkLogin.insert({ uid, true });
		return ret;
	}
	else
	{
		int ret=0;
		bool bst=false;
		if (checkLogin[uid] == bst)
		{
			checkLogin[uid] = true;
			return ret;
		}
		 
		return 1;
	}

	return 0;
}

void upload_File(string fpath, string gid, int cst, string cid)
{
	
	
	 if (gMembers[gid].find(cid) == gMembers[gid].end())
	{
		write(cst, "No client Present", 18);
		appendLog("No client Present\n");
	}
else	if (!pathExist(fpath))
	{
		write(cst, "No file exists", 14);
		appendLog("No file exists\n");
	}
	else if (gMembers.find(gid) == gMembers.end())
	{
		write(cst, "No Group Exists", 16);
		appendLog("No Group Exists\n");
	}
	else
	{
		 
		write(cst, "Uploading", 9);
		appendLog("Uploading\n");
		char fbuff[524288] = { 0 };
		 
		if (read(cst, fbuff, 524288))
		{
			 
			string str="error";
			if (string(fbuff) == str) return;
          
			vector<string> cfd ;
			cfd= seperateString(string(fbuff), "*$*");
			string name ;
			name= seperateString(string(cfd[0]), "/").back();
            
			string hash = "";
			
			size_t i = 4;
			 
			int mysize=cfd.size();
			while(i<mysize)
			{
			 
				hash+= cfd[i];
			 
				if (i != cfd.size() - 1) {
				hash += "*$*";
				}
				i=i+1;
			}
			 
			pieceWise[name] = hash;
int ki=1;
 if(uploadList[gid].find(name) == uploadList[gid].end())
			{
				uploadList[gid].insert({ name,
					{
						cid
					} });
			}
			else if (uploadList[gid].find(name) != uploadList[gid].end())
			{
				ki=1;
				uploadList[gid][name].insert(cid);
			}
			

			curFilesize[name] = cfd[2];
           ki=1;
			write(cst, "Uploaded", 8);
			appendLog("Uploaded");
		}
	}
}

void downloadFile(string gid, string name, string fpath, int cst, string cid)
{
 if (gMembers[gid].find(cid) == gMembers[gid].end())
	{
		write(cst, "No client Present", 18);
		appendLog("No client Present\n");
	}
	
	else if (gMembers.find(gid) == gMembers.end())
	{
		write(cst, "No Group Exists", 16);
		appendLog("No Group Exists\n");
	}
	else if (!pathExist(fpath))
	{
		write(cst, "No file exists", 14);
		appendLog("No file exists");
	}
	else
	{
		 
		char fbuff[524288] = { 0 };
		write(cst, "Downloading", 12);
       appendLog("Downloading\n");
		if (read(cst, fbuff, 524288))
		{
			 
			vector<string> cfd;
cfd			= seperateString(string(fbuff), "*$*");
            
			string mr = "";
		 
			
				 if(uploadList[gid].find(cfd[0]) == uploadList[gid].end())
			{
				write(cst, "File not found", 14);
				 appendLog("File not found\n");
			}
			
			else if (uploadList[gid].find(cfd[0]) != uploadList[gid].end())
			{
			 
				for (auto i: uploadList[gid][cfd[0]])
				{
			 
					if (checkLogin[i])
					{
						mr =mr+ convertPort[i] ;
						mr=mr+ "*$*";
					}
				 
				}
         
				mr =mr+ curFilesize[cfd[0]];
				mr=mr+"&&&";
				mr=mr+pieceWise[cfd[0]];
				
				write(cst, &mr[0], mr.size());// size not length
				 
				uploadList[gid][name].insert(cid);
			}
		
		}
	}
}

//connection of clients using threads and handle commands


void connection(int cst)
{
	string cid="";
	string cgid="";
	string current="";
	string input="";
	cout << "Thread created" << to_string(cst) << endl;
	appendLog("Thread created"+to_string(cst)+"\n");

	while (1)
	{
		char buff[2048] = { 0 };
		vector<string> iarr;
		int z=0;
		if (read(cst, buff, 2048) <= z)
		{
			int ki=1;
			checkLogin[cid] = false;
			close(cst);
			break;
		}

		input = string(buff);
		cout << "Request from client " ;
		cout<< input ;
		cout<< endl;
		appendLog("Request from client "+input+"\n");
		stringstream stringarray(input);

		while (stringarray >> current)
		{
			iarr.push_back(current);
		}

		if (iarr[0] == "create_user")
		{
			int ki=1;
			if (iarr.size() == 3&&ki==1)
			{
				string uid;
				uid=iarr[1];
				string password;
				ki=1;
				password=iarr[2];
				int ans;
				 if(login.find(uid) != login.end())
				{
					ans = -1;
				}
				else if (login.find(uid) == login.end())
				{
					login.insert({ uid, password });
					ans = 0;
				}
				
	 if(ans!=-1)
				{
					write(cst, "Account Sucessfully Created", 28);
					 appendLog("Account Sucessfully Created\n");
				}
			else	if (ans == -1)
				{
					write(cst, "User already exists", 20);
					 appendLog("User already exists\n");
				}
			
			}
			else if(iarr.size() != 3)
			{
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
			}
		}
		else if (iarr[0] == "login")
		{
			int ki=1;

			if (iarr.size() == 3&&ki==1)
			{
				char lbuff[1024];
				int ans ;
				string cca;
				cid = iarr[1];
				
				
				ans= validateLogin(iarr[1], iarr[2]);

				 if (ans == 0)
				{
					write(cst, "Login Successful", 17);
					 appendLog("Login Successful\n");
					read(cst, lbuff, 1024);
					cca = string(lbuff);
					convertPort[cid] = cca;
				}

				else if (ans == 1)
				{
					 ki=1;
					write(cst, "User has already logged in", 27);
					 appendLog("User has already logged in\n");
				}
				
				else
				{
					write(cst, "User_id/Password is incorrect", 30);
					 appendLog("User_id/Password is incorrect\n");
				}
			}
			else if(iarr.size() != 3)
			{
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
			}
		}
		else if (iarr[0] == "logout")
		{

			bool v=false;
			checkLogin[cid] = v;
			write(cst, "Logout Successful", 18);
			appendLog("Logout Successful\n");
		}
		else if (iarr[0] == "upload_file")
		{
			int ki=1;
			if (iarr.size() != 3)
			{
				ki=1;
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
				
			}

			else if(iarr.size() == 3){
				ki=1;
				upload_File(iarr[1], iarr[2], cst, cid);
			}
		}
		else if (iarr[0] == "download_file")
		{
			int ki=1;
			if (iarr.size() != 4&&ki==1)
			{
				ki=0;
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
			}
			else if(iarr.size() == 4){
			downloadFile(iarr[1], iarr[2], iarr[3], cst, cid);
			}
		}
		else if (iarr[0] == "accept_request")
		{
			int ki=1;
			if (iarr.size() != 3)
			{
				ki=1;
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
			}

			else if(iarr.size() == 3)
			{
				int ki=1;
			string grid ;
			grid= iarr[1];
			string uid ;
			uid= iarr[2];
			 if (adminGroups.find(grid)->second != cid)
			{
				write(cst, "You are not admin", 17);
				 appendLog("You are not admin\n");
			}
			else if (gRequests[grid].size() == 0)  
			{
				write(cst, "No requests pending", 20);
				 appendLog("No requests pending\n");
			}

		else if (adminGroups.find(grid) == adminGroups.end())
			{
				write(cst, "No group found", 13);
				 appendLog("No group found\n");
			}
			 
			else
			{
				ki=1;
				write(cst, "Request accepted.", 18);
				 appendLog("Request accepted.");
				gMembers[grid].insert(uid);
				gRequests[grid].erase(uid);
			}
			}
		}
		else if (iarr[0] == "create_group")
		{
			int ki=1;
			if (iarr.size() != 2)
			{
				ki=1;
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
			}

			else if(iarr.size() == 2)
			{
			string grid ;
			grid= iarr[1];
			int res;
			res=0;
			for (string s: groupList)
			{
				 
				if (grid==s&&ki==1)
				{
					write(cst, "Group Already Exists", 21);
					 appendLog("Group Already Exists\n");
					res=1;
				}
				 
			}
   
    
			if(res==0&&ki==1)
			{
			groupList.push_back(grid);
			adminGroups.insert({ grid, cid });
			gMembers[grid].insert(cid);
			cgid = iarr[1];
			ki=1;
			write(cst, "Group Successfully Created", 27);
			 appendLog("Group Successfully Created\n");
			}
			
		 
			}

		}
		
		 
		else if (iarr[0] == "join_group")
		{
			int ki=1;
			if (iarr.size() != 2)
			{
				write(cst, "Invalid Arguments", 18); 
				appendLog("Invalid Arguments\n");
			}

			else if(iarr.size() == 2)
			{
			string grid = iarr[1];
			 if (gMembers[grid].find(cid) != gMembers[grid].end())
			{
				write(cst, "You are already present in group", 32);
				appendLog("You are already present in group\n");
			}
			else if (adminGroups.find(grid) == adminGroups.end())
			{
				write(cst, "Wrong Group ID", 14);
				appendLog("Wrong Group ID\n");
			}
			
			else
			{
				ki=1;
				gRequests[grid].insert(cid);
				write(cst, "Group request successfully sent", 31);
				appendLog("Group request successfully sent\n");
			}
			}
		}
		else if (iarr[0] == "list_requests")
		{
			int ki=1;
			if (iarr.size() != 2 && ki==1)
			{
				write(cst, "Invalid Arguments", 18);
				appendLog("Invalid Arguments\n");
			}

			else if(iarr.size() == 2)
			{
			string grid ;
			grid= iarr[1];
			string requests ;
			requests= "";

			if (adminGroups.find(grid) == adminGroups.end())
			{
				write(cst, "No group found*$*", 18);
				appendLog("No group found*$*\n");
			}

 if (gRequests[grid].size() == 0)
			{
				write(cst, "No requests*$*", 15);
				appendLog("No requests*$*\n");
			}
			
			else if (adminGroups[grid] != cid)
			{
				write(cst, "You are not admin*$*", 20);
				appendLog("You are not admin*$*\n");
			}
			
			else
			{
			auto i = gRequests[grid].begin(); 
				 
				while(i != gRequests[grid].end())
				{
					requests += string(*i) + "*$*";
					i++;
				}

				write(cst, &requests[0], requests.length());

			}
			}
		}
		else if (iarr[0] == "leave_group")
		{
			int ki=1;
			if (iarr.size() != 2)
			{
				ki=1;
				write(cst, "Invalid Arguments", 18);
				appendLog("Invalid Arguments\n");
			}
			else if(iarr.size() == 2)
			{
			string grid;
			grid=iarr[1];
		
			
			 if (gMembers[grid].find(cid) != gMembers[grid].end())
			{
				int ans;
				ans=0;
				 if(adminGroups[grid]== cid)
				{
				write(cst, "You are admin, you cannot leave", 32);
				}
				
				else if (adminGroups[grid]!= cid)
				{
				gMembers[grid].erase(cid);
				//cout<<"";
				write(cst, "Group left succesfully", 23);
				appendLog("Group left succesfully\n");
				int vart=0;
				}
				
			}
				else if (adminGroups.find(grid) == adminGroups.end())
			{
				write(cst, "No group found", 14);
				appendLog("No group found\n");
			}
			else
			{
			 
				write(cst, "You are not part of group", 25);
				appendLog("You are not part of group\n");
			}
			}
		}
		else if (iarr[0] == "list_groups")
		{  
			 
			if (iarr.size() != 1)
			{
			 
				write(cst, "Invalid Arguments", 18);
			 appendLog("Invalid Arguments\n");
			}

			else if (groupList.size() == 0)
			{
				write(cst, "No groups avail*$*", 18);
				 appendLog("No groups avail*$*\n");
			}

			else
			{
			string g = "";
			int i = 0; 
		 
		int mysize=groupList.size();
		while(i<mysize)
			{  
				 
				g += groupList[i] + "*$*";
				i++;
			}
		  
			write(cst, &g[0], g.length() + 1);
			}

		}
		else if (iarr[0] == "list_files")
		{
			int ki=1;
			if (iarr.size() != 2)
			{
				ki=1;
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
			}

			else if(iarr.size() == 2)
			{
			string grid ;
			grid= iarr[1];
			
			 if (uploadList[grid].size() != 0)
			{
				string lfr ;
				lfr= "";
				for (auto i: uploadList[grid])
				{
					lfr =lfr+ i.first ;
					lfr=lfr+ "*$*";
				}

				write(cst, &lfr[0], lfr.length());
				 

			}
			
			else if (adminGroups.find(grid) == adminGroups.end())
			{
				write(cst, "No group found*$*", 18);
					 appendLog("No group found*$*\n");
			}
			
			else
			{
				write(cst, "No files found*$*", 18);
				 appendLog("No files found*$*\n");
			}
			}
		}

		else if (iarr[0] == "show_downloads")
		{
			write(cst, "Downloads", 10);
			 appendLog("Downloads\n");
		}

		else if (iarr[0] == "stop_share")
		{
			int ki=1;
			if (iarr.size() != 3)
			{
				// write(cst, "Invalid Arguments", 17);
				write(cst, "Invalid Arguments", 18);
				 appendLog("Invalid Arguments\n");
			}
			else if (iarr.size() == 3)
			{
			string grid = iarr[1];
			string file_name = iarr[2];
			
			 if (uploadList[grid].find(file_name) != uploadList[grid].end())
			{
				uploadList[grid][file_name].erase(cid);
				//write(cst, "Stopped sharing the file from group", 35);
				write(cst, "Stopped sharing the file from group", 36);
				 appendLog("Stopped sharing the file from group\n");
                int z=0;
				 
				if (uploadList[grid][file_name].size() == z &&ki ==1)
				{
					uploadList[grid].erase(file_name);
				}
				 ki=1;
			}
			
			else if (adminGroups.find(grid) == adminGroups.end())
			{
				//write(cst, "No group found", 13);
				write(cst, "No group found", 15);
				 appendLog("No group found\n");
			}
			
			else
			{
				write(cst, "No file shared in the group", 28);
				 appendLog("No file shared in the group\n");
			}
			}
		}
		
		else
		{
			write(cst, "Invalid Command", 16);
			 appendLog("Invalid Command\n");
		}
	}

	close(cst);
}


int main(int argc, char *argv[])
{ 


 
fpointer = fopen("logFile.txt", "a");

 appendLog("Tracker logs started\n");


	pthread_t close_thread;
	pair<int,string> p;
	p=getIPAndPortFromFile(argv[1]);
	tracker_ip = p.second;
	tracker_port = p.first; 
	 
	int opt = 1;
	int ki=0;
	struct sockaddr_in saddr;
	int addrlen = sizeof(saddr);
int sid;
sid = socket(AF_INET, SOCK_STREAM, 0);
	if (sid == 0)
	{
		 
		cout<<"Socket creation failed"<<endl;
		appendLog("Socket creation failed\n");
		return -1;
	}

	cout << "Tracker socket created"<<endl;

appendLog("Tracker socket created\n");


	if (setsockopt(sid, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		 
		cout<<"Setsockopt of tracker"<<endl;
		appendLog("Setsockopt of tracker\n");
		return -1;
	}
saddr.sin_port = htons(tracker_port);
	saddr.sin_family = AF_INET;
	 
	if (inet_pton(AF_INET, &tracker_ip[0], &saddr.sin_addr) <= 0)
	{
	 
		cout<<endl<<"Invalid address/address not supported"<<endl;
		appendLog("Invalid address/address not supported\n");
		return -1;
	}
    
	if (bind(sid, (SA*) &saddr, sizeof(saddr)) < 0)
	{
	 
		cout<<"Binding failed"<<endl;
		appendLog("Binding failed\n");
		return -1;
	}

 
	
	cout<<endl<<"Binding completed"<<endl;
	appendLog("Binding completed\n");
	//queue size is 5
	if (listen(sid, 5) < 0)
	{
		 
		cout<<"Listen failed"<<endl;
		appendLog("Listen failed\n");
		return -1;
	}

	 
	
	cout<<"Tracker Listening for clients"<<endl;
appendLog("Tracker Listening for clients\n");


	vector<thread> mythreads;
   ki=ki+1;
	//checks for command quit
	if (pthread_create(&close_thread, NULL, closeServer, NULL) == -1)
	{
	 
	cout<<"pthread creation error"<<endl;
	appendLog("pthread creation error\n");

		return -1;
	}
int cst;
	while (1)
	{
		

		if ((cst = accept(sid, (SA*) &saddr, (socklen_t*) &addrlen)) < 0)
		{
		 
			cout<<"Acceptance error during connection of client"<<endl;
			appendLog("Acceptance error during connection of client\n");
		}
    
		
		cout<<endl<<"Connection of client Accepted"<<endl;
		appendLog("Connection of client Accepted\n");

		//adding all clients to tracker in while loop
		mythreads.push_back(thread(connection, cst));
	}

	 
	//making sure all mythreads are executed
	auto itr = mythreads.begin();
	
	 
	while(itr != mythreads.end())
	{
		 
		if (itr->joinable())
		{
		 
			itr->join();
		}
		 
	itr++;
	}

	 
	cout<<"Exit"<<endl;
	appendLog("Exit\n");


	appendLog("Tracker logs finished\n\n");

    fclose(fpointer);

	return 0;
}
