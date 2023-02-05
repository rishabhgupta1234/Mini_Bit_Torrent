#include <bits/stdc++.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>


using namespace std;
  
#define MAX_CHUNK_SIZE 524288
#define SIZE 32768
#define SA struct sockaddr
string FileName, tracker_ip, peer_ip;
int checkLogin = 0, checkSHA = 1;
uint16_t trackerPort, peer_port;
unordered_map<string, string> fileToPath;
int flag=0;


vector<string> seperateString(string str, string f)
{
	string t;
	size_t pos = 0;
	vector<string> res;
	 pos = str.find(f);
	while ( pos != string::npos)// if f is present in the string 
	{
		t = str.substr(0, pos);
		
		res.push_back(t);
		
		str.erase(0, pos + f.length()); 
		
		 pos = str.find(f);
	}

	res.push_back(str); 
	return res;
}

pair<int,string> getIPAndPortFromFile(char* fn){
	string name = string(fn);
    string z;
	z="";
    ifstream input_file(name);

    if (input_file.is_open()) {// if file is successfully opened
		getline(input_file, z);
		vector<string> tip= seperateString(z,":");
	 
		string ip=tip[0];
		int port= stoi(tip[1]);
		 
		
        pair<int,string>  p;
		p= make_pair(port,ip);
		return p;
    }

	else{
		
	 
		cout<<"File cannot be open "<<endl;
		pair<int,string>  p;
		p= make_pair(0,"0.0.0.0");
		return p;
	}
}
 
long long int fileSize(char *route)
{
	 
	long long int size = -1;
	FILE *fpointer = fopen(route, "rb");
 
	size = -1;
	if (fpointer)
	{
		fseek(fpointer, 0, SEEK_END);
		 
		size = ftell(fpointer) ;
		size=size + 1;
		 
		fclose(fpointer);
	}
	else if(fpointer==0)
	{
		 
		cout<<"File not found"<<endl;

		return -1;
	}

	return size;// return size if file is already received 
}


typedef struct peerdetails
{
	string serverPeerIP,filename;
	 
	long long filesize;
} peerdetails;
unordered_map<string, unordered_map<string, bool>> uploadList;
unordered_map<string, string> downloads;
vector<vector<string>> currentChunks;
unordered_map<string, vector<int>> chunkInfo;
vector<string> pieceSha;


void singleHash(string &h, string str)
{
	unsigned char arr[20];
	if (SHA1(reinterpret_cast<const unsigned char *>(&str[0]), str.length(), arr))
	{
	int i=0;
	 
	while(i<20)
		{
			char b[3];
			sprintf(b, "%02x", arr[i] & 0xff);
			h = h+string(b);
			i++;
		}
	}
	else
	{
		 
		cout<<"Error in hashing"<<endl;
	}

	h =h+ "*$*";
}

typedef struct requiredcDetails
{
	long long chunkNum;
	string serverPeerIP,filename,destination; 
} requiredcDetails;

void Vector_Of_Bits(string name, long long int l, long long int h, int lastvar)
{

 if(lastvar!=0)
	{
	  int z=h - l + 1;
	  
		vector<int> v(z, 1);
		chunkInfo[name] = v;
	}
	else if (lastvar == 0)
	{
		chunkInfo[name][l] = 1;
	}
	
}



string peerConnection(char *PeerP, char *pl, string temp)
{
	uint16_t pp ;
	pp= stoi(string(pl));
	
	struct sockaddr_in cpsa;// creating the structure 
	int sp = 0;
	sp = socket(AF_INET, SOCK_STREAM, 0);
	if (sp < 0)
	{
		cout<<endl<<" Socket creation error "<<endl;
		
		string ret ="error";
			return ret;
	}
	 
	//initialize the environment for sockaddr structure
	cpsa.sin_port = htons(pp);
	cpsa.sin_family = AF_INET;
	
	 
	if (inet_pton(AF_INET, PeerP, &cpsa.sin_addr) < 0)
	{
		string str="ERROR:";
		 
		cout<<str;
		cout<<endl;
		perror("Peer Connection Error(INET)");
	}
	 
	if (connect(sp, (struct sockaddr *)&cpsa, sizeof(cpsa)) < 0)
	{
		string str="ERROR:";
		 
		cout<<str<<endl;
		perror("Peer Connection Error");
	}
	 

	string match = seperateString(temp, "*$*").front();

	
	
	  if (match == "current_chunk")
	{
		 
		if (send(sp, &temp[0], strlen(&temp[0]), MSG_NOSIGNAL) == -1)
		{
		 
		cout<<"Error in socket reading in current chunk"<<endl;
		
		string ret ="error";
			return ret;
		}
		 
		vector<string> tik = seperateString(temp, "*$*");
		string hash = "";
		long long int chunkNum ;
		string dpath = tik[3];
		char *fpath = &dpath[0];
		  chunkNum = stoll(tik[2]);
		int number,techn=0;
		 
		char cbuff[MAX_CHUNK_SIZE];

		string fuc = "";
		while (techn < MAX_CHUNK_SIZE)
		{
			//reading the socket received and storing in the current_buff
			number = read(sp, cbuff, MAX_CHUNK_SIZE - 1);
			if (number <= 0)
			{
				break;
				 
			}

			cbuff[number] = 0;
			fstream outline(fpath, std::fstream::in | std::fstream::out | std::fstream::binary);
			 
			outline.seekp(chunkNum * MAX_CHUNK_SIZE + techn, ios::beg);
			 
			outline.write(cbuff, number);
			outline.close();
			 

			fuc =fuc+ cbuff;
			techn = techn+number;
			bzero(cbuff, MAX_CHUNK_SIZE);
		}

		singleHash(hash, fuc);
		 
		int p=0;
		while(p<3){
			hash.pop_back();
		p++;
		}

		if (hash != pieceSha[chunkNum])
		{
			checkSHA = 0;
		}

		vector<string> fname = seperateString(string(fpath), "/");
		string file_name;
		file_name = fname.back();
		Vector_Of_Bits(file_name, chunkNum, chunkNum, 0);
		string ret="done";
		
		return ret;
	}
	else if (match == "current_path_file")
	{
		if (send(sp, &temp[0], strlen(&temp[0]), MSG_NOSIGNAL) == -1)
		{
			 
			cout<<"Error in socket peer in file path"<<endl;
			string ret="error";
			return ret;
		}
		 
		char rback[10240] = {0};
		if (read(sp, rback, 10240) < 0)
		{
			 
			cout<<"Error in socket reading in current path"<<endl;
			string ret="error";
			return ret;
		}
		 
		fileToPath[seperateString(temp, "*$*").back()] = string(rback);
	}
	else if (match == "current_chunk_vector_details")
	{
		if (send(sp, &temp[0], strlen(&temp[0]), MSG_NOSIGNAL) == -1)
		{
			 
			cout<<"Error in socket reading in current chunk vector"<<endl;
		string ret="error";
			return ret;
		}
		 
		char rback[10240] = {0};
		//reading the socket and storing in rback
		if (read(sp, rback, 10240) < 0)
		{
		 
			cout<<"Error in socket reading in current chunk vector"<<endl;
			string ret="error";
			return ret;
		}
		 
		close(sp);
		 
		return string(rback);

		 
	}
	

	close(sp);
	string ret="done";
			return ret;
}

 
string combinehash(char *fpath)
{
	long long asize = fileSize(fpath);// return the total size of the file 
	vector<string> v;
	string g = "";
	string str = "";
 
	int custom, curt;
	if (asize == -1)// file is not present 
	{
	string ret="XYZ";
	
		return ret;
	}

	FILE *fp = fopen(fpath, "r");// open the file in read mode 
	int cseg = (asize / MAX_CHUNK_SIZE) ;// totoal number of chunks 
	cseg++;
	int i;
	char varpoll[SIZE + 1];

	if (fp)
	{
	i=1;
		 
		while(i<=cseg)
		// calling singleHash function for all the number of chunks 
		{
			custom = 0;
			while (custom < MAX_CHUNK_SIZE && (curt = fread(varpoll, 1, min(SIZE - 1, MAX_CHUNK_SIZE - custom), fp)))//reads the block of data from the stream
			{
				 
			 
				varpoll[curt] = '\0';
				custom =custom+ strlen(varpoll);

				str =str+ varpoll;
				memset(varpoll, 0, sizeof(varpoll));
			}
			 
			singleHash(g, str);
			
			i=i+1;
		}

		fclose(fp);
		 
	}
	else
	{
		 
		cout<<"File not found"<<endl;
	}
     int p=0;
	 
	while(p<3)
	{
		g.pop_back();
		p=p+1;
	}
	return g;
}

int upload_file(vector<string> iarr, int sock)
{
	 
	 
	char *filepath = &iarr[1][0];
    
	string filename ;
	filename= seperateString(string(filepath), "/").back();//returning the last element 
	 
	
	 if(uploadList[iarr[2]].find(filename) == uploadList[iarr[2]].end())
	{
		uploadList[iarr[2]][filename] = true;// setting file is present 
		 
		fileToPath[filename] = string(filepath);// storing the path of file 
	}
	
	else if (uploadList[iarr[2]].find(filename) != uploadList[iarr[2]].end())
	{
	 
		cout << "File already uploaded" << endl;
		if (send(sock, "error", 5, MSG_NOSIGNAL) == -1)
		{
		 
			cout<<"Error in uploading"<<endl;
			return -1;
		}
		 
		return 0;
	}
	

	string pieceHash ;
	pieceHash=combinehash(filepath);// finding the combine hash of the file store at location filepath
	if (pieceHash == "XYZ"){
		return 0;
		}
	string filehash;
string tyu;
	ostringstream charbuf;
	ifstream in(filepath);
	charbuf << in.rdbuf();
	string contents = charbuf.str();
	


	unsigned char current_file_buffer_hash[32];
	tyu	= "";
	if (!SHA256(reinterpret_cast<const unsigned char *>(&contents[0]), contents.length(), current_file_buffer_hash))
	{
	 
		cout<<"Error in hashing"<<endl;
	}
	else
	{
	int i=0;
		 
		while(i<32)
		{
			char cur_buff[3];
			sprintf(cur_buff, "%02x", current_file_buffer_hash[i] & 0xff);
			tyu += string(cur_buff);
			i=i+1;
		}
	}
string fdet ;
filehash	= "";
	filehash = tyu;
	string filesize = to_string(fileSize(filepath));// calculating the file size store at filepath 
fdet= "";
	fdet =fdet+ string(filepath) + "*$*";
 
	fdet =fdet+ string(peer_ip) + ":" + to_string(peer_port) + "*$*";// combining ip address and port address
	 
	fdet =fdet+ filesize + "*$*";
	fdet =fdet+ filehash + "*$*";
	 
	fdet =fdet+ pieceHash;// adding the hash value also
	 

	if (send(sock, &fdet[0], strlen(&fdet[0]), MSG_NOSIGNAL) == -1)// sends the data to the sock socket
	{
		 
		cout<<"Error in uploading"<<endl;
		return -1;
	}

	char rback[10240] = {0};
	read(sock, rback, 10240);
	cout << rback;
	cout<<endl;
 int g=MAX_CHUNK_SIZE + 1;
 
	Vector_Of_Bits(filename, 0, stoll(filesize) / g, 1);

	return 0;
}

void tfunc(peerdetails *pfd)
{
 
	string sm ;
	sm= "current_chunk_vector_details*$*" + string(pfd->filename);
	vector<string> spa ;
	spa= seperateString(string(pfd->serverPeerIP), ":");
	 
	string response ;
	response= peerConnection(&spa[0][0], &spa[1][0], sm);
	 
	
	size_t i;
	i = 0;
	int mysize=currentChunks.size();
	 
	while(i<mysize)
	{
		string strt="";
    
	 if(response[i] != '1')
		{
			strt=strt+"error";
			 
		}
		else if (response[i] == '1')
		{
			if(strt=="exit"){
			 break;
			 }
			currentChunks[i].push_back(string(pfd->serverPeerIP));
			 
		}
		i=i+1;
	}

	delete pfd;
}

void tfunc2(requiredcDetails *ar)
{
 string name ;
 name= ar->filename;//getting filename from structure
	vector<string> pp ;
	pp= seperateString(ar->serverPeerIP, ":");
	
	string d ;
	d= ar->destination;// geeting destination from structure
	long long int cvar;
cvar	= ar->chunkNum;//getting checknum from structure
	string s= to_string(cvar);	
	string sm ;
	sm= "current_chunk*$*" + name ;
	sm=sm+ "*$*" + to_string(cvar) ;
sm=sm+	"*$*" + d;
	s=s+" ";
	peerConnection(&pp[0][0], &pp[1][0], sm);
	string str="error";
	if(s==str){
	 return;
	 }
	delete ar;
	 
	return;
}

void piecewiseAlgorithm(vector<string> iarr, vector<string> peers)
{
	string fname;
	vector<thread> threads;

	long long int fsize;
fsize	= stoll(peers.back());
	peers.pop_back();
	long long int cg ;
	cg= (fsize / MAX_CHUNK_SIZE) ;
	cg=cg+ 1;// calculating the number of segments
	currentChunks.clear();
	currentChunks.resize(cg);

	vector<thread> threads2;
	fname=iarr[0];

size_t i = 0; 
int mysize=peers.size();
	 
	while(i<mysize)
	{
		peerdetails *pf = new peerdetails();
		 
		 pf->serverPeerIP = peers[i];// assigning the IP
		pf->filesize = cg;// assigning the size to the structure variable
		 
		pf->filename = iarr[2];//assigning to the file name 
		threads.push_back(thread(tfunc, pf));
		i=i+1;
	}
	
	
	
	 
auto it = threads.begin(); 
	 
	while(it != threads.end())
	{
		if (it->joinable()){
			it->join();
			}
			it++;
	}
 i = 0; 
 mysize=currentChunks.size();
 
while(i < mysize)
	{
		int temp = 0;
		int mysize=currentChunks[i].size();
		if (mysize == 0)
		{
			if(temp<0){
			break;
			}
			cout << "All parts of the file are not available" ;
			cout<< endl;
			temp =temp+ 1;
			return;
		}
		temp = 0;
		i=i+1;
	}
	 

	threads.clear();
	srand((unsigned)time(0));
	long long int recs ;
	recs= 0;
	 
	string desp = iarr[3] + "/" ;
	desp=desp+ iarr[2];
	 
	FILE *fp ;
	fp= fopen(&desp[0], "w");
	if (fp == 0)
	{
	 
	cout<<"The file already exist"<<endl;
		fclose(fp);
		return;
	}
	fclose(fp);
	 
	string ss(fsize, '\0');

	int t = 0;
	fstream in(&desp[0], ios::out | ios::binary);
    fname=fname+to_string(t);
	in.write(ss.c_str(), strlen(ss.c_str()));
	in.close();
	 
	chunkInfo[iarr[2]].resize(cg, 0);
	checkSHA = 1;

	vector<int> tmp(cg, 0);
	chunkInfo[iarr[2]] = tmp;

	string ptgfp;

	while (recs < cg)
	{
		long long int rpc;
		while (1)
		{
			rpc = rand() % cg;
			if (chunkInfo[iarr[2]][rpc] == 0){
				break;
				}
		}

		long long int pwts;
      pwts	= currentChunks[rpc].size();
		string rpeer;
		rpeer = currentChunks[rpc][rand() % pwts];
		string d;
		d = iarr[3] + "/" ;
		d=d+ iarr[2];
		string fyz=d;
		 
		requiredcDetails *req = new requiredcDetails();
		req->destination = fyz;
		
		 
		req->chunkNum = rpc;
		req->serverPeerIP = rpeer;
		req->filename = iarr[2];
		
		chunkInfo[iarr[2]][rpc] = 1;
         
		threads2.push_back(thread(tfunc2, req));//passing tfunc2 along with thread 
		recs=recs+1;
		string sio="break";
        if(fname==sio){
		  break;
		  }
		
		ptgfp = rpeer; 
	}
auto itr = threads2.begin();
	 
	while(itr!=threads2.end())
	{
		 
		if (itr->joinable()){
			itr->join();
		}
     
	 itr++;
	 
	}

	if (checkSHA == 0)
	{
		
		cout << "Download completed, the file may be corrupted"<<endl;
		 
	}
	else
	{
		 
		cout << "Download completed, no corruption detected" << endl;
	}

	downloads.insert({iarr[2], iarr[1]});
     
	vector<string> sadd;
sadd	= seperateString(ptgfp, ":");

	 
	peerConnection(&sadd[0][0], &sadd[1][0], "current_path_file*$*" + iarr[2]);
	 fname=fname+"";
	return;
}





int downloadFILE(vector<string> iarr, int sock)
{
	if (iarr.size() != 4)
	{
		return 0;
	}
 
	 
	string fdet = "";
	 
		 
	fdet = fdet+iarr[2] + "*$*";
	fdet =fdet+ iarr[3] + "*$*";
	fdet =fdet+ iarr[1];
	 
	// sends data on the socket
	if (send(sock, &fdet[0], strlen(&fdet[0]), MSG_NOSIGNAL) == -1)
	{
		 
		
		cout<<"Error in downloading"<<endl;
		return -1;
	}

	char repb[524288] = {0};
	read(sock, repb, 524288);//return message is stored in reply back 
	 
string str="File not found";
	if (string(repb) == str)
	{
		 
		cout<<str<<endl; 
		return 0;
	}
	 
	vector<string> ran ;
	ran= seperateString(string(repb), "&&&");

	vector<string> pwf ;
	pwf= seperateString(ran[0], "*$*");// passing IP

	vector<string> tmp ;
	tmp= seperateString(ran[1], "*$*");//passing port
	 
	pieceSha = tmp;

	piecewiseAlgorithm(iarr, pwf);
	return 0;
}




int connection(vector<string> iarr, int sock)//after this all function are called
{
	char rebac[10240];//declaring the character array 
    //erases the data in the n bytes of the memory starting at the location pointed to by s
	bzero(rebac, 10240);
    //read the socket and store in the rebac 
	read(sock, rebac, 10240);
	string str="Invalid Arguments";
	if (string(rebac) == str)
	{
		cout<<"Here";
		cout << rebac << endl;
		return 0;
	}

	

	if (iarr[0] == "login")
	{

		flag=1;
	str="Login Successful";
	
	 if(string(rebac) != str)
		{
			cout << rebac << endl;
		}
		else if (string(rebac) == str)
		{
			checkLogin = 1;
			cout << rebac << endl;
			string padd ;
			padd= peer_ip + ":" ;
			padd=padd+to_string(peer_port);
            //providing the IP address and port number to sock 
			write(sock, &padd[0], padd.length());
		}
		
	}
	else if (iarr[0] == "logout")
	{
		checkLogin = 0;
		flag=0;
		cout << rebac ;
		cout<< endl;
	}
	else if (iarr[0] == "create_group")
	{
		cout << rebac ;
		cout<< endl;
	}
	else if (iarr[0] == "accept_request")
	{
		cout << rebac;
		cout << endl;
	}
	else if (iarr[0] == "join_group")
	{
		cout << rebac ;
		cout<< endl;
	}
	else if (iarr[0] == "leave_group")
	{
		cout << rebac;
		cout << endl;
	}
	
	
	else if (iarr[0] == "upload_file")
	{
	str="Uploading";
		if (string(rebac) != str)
		{
			cout << rebac << endl;
			return 0;
		}

		cout << rebac << endl;
		return upload_file(iarr, sock);
	}
	else if (iarr[0] == "download_file")
	{
	str="Downloading";
		if (string(rebac) != str)
		{
			cout << rebac << endl;
			return 0;
		}

		cout << rebac << endl;
		 
		if (downloads.find(iarr[2]) != downloads.end())// file is already present in downloads array
		{
           
			cout << "File already downloaded" << endl;
		 
			return 0;
		}
        
		return downloadFILE(iarr, sock);
	}
	else if (iarr[0] == "list_files")
	{
		vector<string> lof ;
		lof= seperateString(string(rebac), "*$*");
	 
	size_t i = 0; 
	 
	int mysize=lof.size() - 1;
	while(i < mysize)
		{
			cout << lof[i] << endl;
			i=i+1;
		}
	}
	
	else if (iarr[0] == "list_requests")
	{
		vector<string> req;
	req	= seperateString(string(rebac), "*$*");
		 
		
		size_t i = 0; 
	 
		int mysize=req.size() - 1;
		while(i < mysize)
		{  
		 
			cout << req[i] << endl;
			i=i+1;
		}
	}
	
	else if (iarr[0] == "stop_share")
	{
		cout << rebac ;
		cout<< endl;
		uploadList[iarr[1]].erase(iarr[2]);
	}
	else if (iarr[0] == "list_groups")
	{
		 
		 
		vector<string> grps = seperateString(string(rebac), "*$*");
		 
		size_t i = 0;
	 
		int mysize=grps.size() - 1;
		while(i < mysize)
		{ 
           
			cout << grps[i] ;
			cout<<endl;
			i=i+1;
		}
	}
	else if (iarr[0] == "show_downloads")
	{
	 
		cout << rebac << endl;
		 

         if(downloads.size()==0){ 
			cout<<"Download is empty"<<endl;
		 }
		 else{ 

		for (auto p : downloads)
		{
		 
			cout << "[C] ";
cout			<< p.second ;
cout<< " " << p.first ;
cout<< endl;
		}
		 }
	}
	else
	{
		cout << rebac << endl;
	}

	return 0;
}




void handleconnection(int cst)
{
	 
	char icl[1024] = {0};
//read() call reads data on a socket with descriptor fs and stores it in a buffer
	if (read(cst, icl, 1024) <= 0)
	{
		close(cst);
		return;
	}
  
	vector<string> iarr ;
	iarr= seperateString(string(icl), "*$*");
	 


 if (iarr[0] == "current_path_file")
	{
		string cfp;
cfp		= fileToPath[iarr[1]];
        //this function writes data from a currentfilepath on a socket  
		write(cst, &cfp[0], cfp.length());
	}

	else if (iarr[0] == "current_chunk")
	{
		int f = 0;
		string sent = "";
		long long int cnum;
	cnum= stoll(iarr[2]);
		string cfp ;
		cfp= fileToPath[iarr[1]];
		char *fp ;
		fp= &cfp[0];
		 
		std::ifstream file_pointer(fp, std::ios::in | std::ios::binary);
		 
		 
		file_pointer.seekg(cnum * MAX_CHUNK_SIZE, file_pointer.beg);
		char sto[MAX_CHUNK_SIZE] = {0};
		file_pointer.read(sto, sizeof(sto));
		int count ;
		count= file_pointer.gcount();
		f = send(cst, sto, count, 0);
		if (f == -1)
		{
			 
			cout<<"Error in sending file"<<endl;
			exit(1);
		}

		file_pointer.close();
	}
	else if (iarr[0] == "current_chunk_vector_details")
	{
	 
		string name = iarr[1];
		string m = "";
		vector<int> ch ;
		ch= chunkInfo[name];
		for (int i : ch)
		{
			m =m+ to_string(i);
		}
		 
		char *temp = &m[0];
        // this function sends data on the socket
		send(cst, temp, strlen(temp), 0);
	}
	
	close(cst);
	return;
}

void *serverHelper(void *arg)
{
	 
	int sid;
	struct sockaddr_in serverAddr;// making an structure which contain ip,port,etc
	
	int alen = sizeof(serverAddr);
    sid = socket(AF_INET, SOCK_STREAM, 0);//initializing the socket 

	if (sid == 0)
	{
		 
		cout<<"Socket creation failed"<<endl;
		exit(EXIT_FAILURE);
	}
int o = 1;
	 
    // it is used to set the socket options
	if (setsockopt(sid, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &o, sizeof(o)))
	{
 
 cout<<"Setsockopt of peer server"<<endl;
	 
		exit(EXIT_FAILURE);
	}
    //initialize the environment for sockaddr structure
	serverAddr.sin_family = AF_INET;// assigning the family to server object variable
    serverAddr.sin_port = htons(peer_port);// assigning the port address to server object variable
	
	
   
	if (inet_pton(AF_INET, &peer_ip[0], &serverAddr.sin_addr) <= 0)
	{
		 
		cout<<endl<<"Invalid address/address not supported "<<endl;
		exit(EXIT_FAILURE);
	}
    
	if (bind(sid, (SA *)&serverAddr, sizeof(serverAddr)) < 0)// bind the socket to local port 
	{
	 
		cout<<"Binding failed"<<endl;
		exit(EXIT_FAILURE);
	}

	 
	cout<<"Binding completed"<<endl;
  
	if (listen(sid, 3) < 0)// listen the request from client (if more than queue the request)
	{ 
		cout<<"Listen failed"<<endl;
		exit(EXIT_FAILURE);
	}

	 
	cout<<"PeerServer Listening for clients"<<endl;
 
    int client_socket;
	vector<thread> threads;
	//creating a new socket descriptor with the same properties as socket
	while (((client_socket = accept(sid, (SA *)&serverAddr, (socklen_t *)&alen))))
	{
       
		threads.push_back(thread(handleconnection, client_socket));
	}
    //If the queue has no pending connection requests, accept() blocks the caller unless socket is in nonblocking mode
	//making sure all threads are executed
	
	auto itr = threads.begin(); 
	 
	while(itr != threads.end())
	{
		 
   
		if (itr->joinable())
		{  
			itr->join();
		}
		else
		{
			 
		}
		itr++;
	}

	close(sid);
	return NULL;
}

int main(int argc, char *argv[]){

if (argc != 3)
	{
		 
		cout<<"Invalid number of argument"<<endl;
		return -1;
	}

    vector<string> v;
	v=seperateString(argv[1],":");
	peer_ip = v[0];
	peer_port = stoi(v[1]);
	pair<int,string> p;
	p=getIPAndPortFromFile(argv[2]);// from tracker_info.txt file 
	trackerPort = p.first;//stoi(argv[4]);
	tracker_ip = p.second;
	



struct sockaddr_in saddr;// structure which contains port,ip,etc info 
	int skt = 0;
	thread t;
	pthread_t thread_server;// thread variable 
    
    skt = socket(AF_INET, SOCK_STREAM, 0);// initialize the socket 
	if (skt < 0)
	{  
		 
	cout<<endl<<"Socket creation error"<<endl;
		return -1;
	}


  
	cout<<"Peer socket created successfully";
	 
    cout<<endl;
// mutliple thread used to create mutliple socket which can be used to establish connection 
	if (pthread_create(&thread_server, NULL, serverHelper, NULL) == -1)//thread is created and serverHelper is passed
	{
		 
	cout<<"pthread error"<<endl;
		return -1;
	}


    saddr.sin_family = AF_INET;// assigning the type to structure variables
	 
	saddr.sin_port = htons(trackerPort);// assigning the local port 

	if (inet_pton(AF_INET, &tracker_ip[0], &saddr.sin_addr) <= 0)
	{
		 
		cout<<endl<<"nvalid address/address not supported "<<endl;
		return -1;
	}

	if (connect(skt, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)//setting connection using socket
	{
		 
		cout<<"Connection failed"<<endl;
		return -1;
	}

while (1)
	{
		string inp;
		string curr;
		getline(cin, inp);
		

		if (inp.size() <= 0){
			continue;
			}

		stringstream stringarray(inp);
        vector<string> iarr;
		while (stringarray >> curr)
		{
			iarr.push_back(curr);
		}

if (checkLogin == 0 && iarr[0] != "login" && iarr[0] != "create_user")
		{
			 
			cout << "Please login / create an account";
			 
			cout<<endl;
			continue;
		}
		
		if ((iarr[0] == "login" && checkLogin == 1 ))
		{
			cout << "You already have one active session" << endl;
			continue;
		}
		if(iarr[0]=="create_user" && flag==1){
			cout<<"You are not allowed to create user"<<endl;
			continue;
		}

		
        //this function sends data on the socket  
		if (send(skt, &inp[0], strlen(&inp[0]), MSG_NOSIGNAL) == -1)
		{
			 
			cout<<"Error from server"<<endl;
			return -1;
		}

		connection(iarr, skt);
	}

	close(skt);
	return 0;

}


 
