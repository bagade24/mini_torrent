#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include<bits/stdc++.h>
#include <unistd.h>
#include<vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include<time.h>


using namespace std;
#define SIZE 1024
/////////////////////////////////////////////////////utilities
string convertToString(char* a, int size)
{
    string s = a;
    return s;
}
///////////////////////////////////////////////////////////////
class TorrentFile{
public:
  string filename;
  int filesize=0;
  multimap<int,string> partsoffile;
  

  TorrentFile()
  {

  }
};

class Group{
public:
  string groupid;
  vector<string> users;
  //map<string,bool> userlogin;
  multimap<string,string> filedetails;
  vector<string> listoffiles;
  multimap<string,string> filepathdetails;
  map<string,string> filesizedetails;
  map<string,string> fileSHAdetails;
  vector<string> pendingrequests;
  //map<string,map<int,vector<string>>> filechunks;
  vector<TorrentFile> chunkdetails;
  int adminid=0;
  Group()
  {

  }
};



class User{
public:
  int new_sock;
  string userid="";
  string command="";
  int clientport;
  
  User()
  {

  }
};

///////////////////////////////////////////////////////
class Server{
public:
////////////////////   DATA SECTION  //////////////////

  char ip[10] = "127.0.0.1";
  int port = 8081;
  int e;
  int opt=1;
  int sockfd, new_sock;
  struct sockaddr_in server_addr, new_addr;
  socklen_t addr_size;
  
//////////////////////////////////////////////////////////////
  string current_user="";

  map<string,string> userdetails;
  vector<Group> groupdetails;
  map<string,bool> logindetails;
  vector<User> clientdetails;
  map<string,int> portdetails;

  ////////////////////////////////////////////////////////////
  Server(int port)
  {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
    perror("[-]Error in socket");
    exit(1);
   }
  printf("[+]Server socket created successfully.\n");
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  }

  void serverbind()
  {
     e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
   if(e < 0) {
     perror("[-]Error in bind");
     exit(1);
   }
  printf("[+]Binding successfull.\n");
  }

  void serverlisten()
  {
  if(listen(sockfd, 10) == 0){
  printf("[+]Listening....\n");
  }else{
  perror("[-]Error in listening");
    exit(1);
  }

  }

void writefile()
{
  addr_size = sizeof(new_addr);
  new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
  write_file(new_sock);
  printf("[+]Data written in the file successfully.\n");
}

void write_file(int sockfd)
{
  int n;
  FILE *fp;
  char filename[] = "recv.txt";
  char buffer[SIZE];

  fp = fopen(filename, "w");
  while (1) {
    n = recv(sockfd, buffer, SIZE, 0);
    if (n <= 0){
      break;
      return;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
  }
  return;
}
int getclientaddr(User &u)
{
  char buffer[SIZE]={0};
  int valread = read( u.new_sock , buffer, 1024);
  int buffer_size = sizeof(buffer) / sizeof(char);
  int val=stoi(buffer);

  return val;
}
void acceptconnection()
{
  addr_size = sizeof(new_addr);
  new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
  
  if(new_sock >=0)
  {
    User u1;
    u1.new_sock=new_sock;
    clientdetails.push_back(u1);
    int clientaddress=getclientaddr(u1);
    u1.clientport=clientaddress;
    thread t1(&Server::acceptconnection,this);
    t1.detach();
  while(1)
  {
    getcommand(u1);
  }
  }
}

void getcommand(User &u)
{
  //cout<<"in get COMMAND"<<endl;
  char buffer[SIZE]={0};
  int valread = read( u.new_sock , buffer, 1024);
  int buffer_size = sizeof(buffer) / sizeof(char);
  string mycommand=convertToString(buffer,buffer_size);
  u.command=mycommand;
  //cout<<mycommand<<endl;
  if(mycommand!="")
  {
    thread t1(&Server::stripcommand,this,std::ref(u));
    t1.detach();
  }
 
}

void stripcommand(User &u)
{
  string mycommand=u.command;
  vector<string> strippedcommands;
  mycommand+=" ";
  string s="";
  
  for(int i=0;i<mycommand.length();i++)
  {
    if(mycommand[i] != ' ')
    {
          s+=mycommand[i];
    }
    else{
      strippedcommands.push_back(s);
      s="";
    }  
  }
  processcommands(strippedcommands,u);
  return;
}

/////////////////////////////////////////Reply to client//////////////////////////////////////////////////
void clientreply(User &u,string reply)
{
   write(u.new_sock, reply.c_str(), reply.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool checkinvalidparameters(vector<string> strippedcommands, int parameters)
{
    if(strippedcommands.size()==parameters)
    {
      return true;
    }
    //cout<<"[-] INVALID ARGUMENTS "<<endl;
    return false;
}

void processcommands(vector<string> strippedcommands,User &u)
{
  
  if(strippedcommands[0]=="create_user")
  {
    if(!checkinvalidparameters(strippedcommands,3))
    {
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
    }

    createuserfunction(strippedcommands,u);
    return;
  }

  if(strippedcommands[0]=="login")
  {
     if(!checkinvalidparameters(strippedcommands,3)) {
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
    }
    loginfunction(strippedcommands,u);
    return;
  }
  if(strippedcommands[0]=="create_group"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,2))
     {
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
     }

    creategroupfunction(strippedcommands,u);
    return;
  }
  if(strippedcommands[0]=="join_group"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,2)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
     }

    joingroupfunction(strippedcommands,u);
    return;
  }
  if(strippedcommands[0]=="leave_group"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,2)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
     }

    leavegroupfunction(strippedcommands,u);
    return;
  }
  if(strippedcommands[0]=="requests"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,3)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
     }

    listpendingrequestsfunction(strippedcommands,u);
    return;
  }

  if(strippedcommands[0]=="accept_request"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,3)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
     }

    acceptrequestfunction(strippedcommands,u);
    return;
  }

  if(strippedcommands[0]=="list_groups"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,1)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
     }

    listallgroupsfunction(u);
    return;
  }

  if(strippedcommands[0]=="logout")
  {
     if(!checkinvalidparameters(strippedcommands,1)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
     }

    logoutfunction(u);
    return;
  }

  if(strippedcommands[0]=="upload_file"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,5)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
  }

    uploadfunction(strippedcommands,u);
    return;
  }

  if(strippedcommands[0]=="list_files"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,2)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
  }

    listfilesfunction(strippedcommands,u);
    return;
  }


  if(strippedcommands[0]=="download_file"&&u.userid!="")
  {
     if(!checkinvalidparameters(strippedcommands,4)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
  }

    downloadfilesfunction(strippedcommands,u);
    return;
  }

  if(strippedcommands[0]=="update_chunk")
  {
     if(!checkinvalidparameters(strippedcommands,4)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
  }

    updatechunkfunction(strippedcommands,u);
    return;
  }

  if(strippedcommands[0]=="stop_sharing")
  {
     if(!checkinvalidparameters(strippedcommands,3)){
      clientreply(u,"[-] INVALID ARGUMENTS\n");
      return; 
  }

    stopsharingfunction(strippedcommands,u);
    return;
  }




   clientreply(u,"[-] INVALID COMMAND\n");
}
/////////////////// USER CREATION ///////////////////////////
bool userexists(string userid)
{
  if(userdetails.find(userid) != userdetails.end())
  {
    return true;
  }
  return false;
}

void createuser(string userid,string userpassword)
{
  userdetails.insert({userid,userpassword});
  //userlogin.insert({userid,false});
 
  //cout<<"[+] USER "<<userid<<" sucessfully created "<<endl;
}

void createuserfunction(vector<string> strippedcommands,User &u)
{
   string userid = strippedcommands[1];
   string userpassword = strippedcommands[2];
    if(userexists(userid))
    {
      clientreply(u,"[-] INVALID COMMAND : USER ALREADY EXISTS \n");
      return;
    }
    createuser(userid,userpassword);
    clientreply(u,"[+] USER "+userid+" sucessfully created \n");
    logindetails.insert({userid,false});
    return;
}

///////////////////   LOGIN    /////////////////////
void loginfunction(vector<string> strippedcommands,User &u)
{
  string userid = strippedcommands[1];
  string userpassword = strippedcommands[2];
  if(!userexists(userid))
  {
    //cout<<"[-] INVALID COMMAND : USER DOES NOT EXIST "<<endl;
    clientreply(u,"[-] INVALID COMMAND : USER DOES NOT EXIST  \n");
    return;
  }

  if(userdetails[userid] == userpassword)
  {
    //current_user=userid;
    u.userid=userid;
    logindetails[u.userid]=true;
   // userlogin[current_user]=true;
    portdetails.insert({userid,u.clientport});
    //cout<<"[+] LOGIN SUCCESSFULLY "<<endl;
    clientreply(u,"[+] LOGIN SUCCESSFULLY \n");
  }
  else{
    //cout<<"[-] LOGIN UNSUCCESSFULLY : WRONG PASSWORD "<<endl;
    clientreply(u,"[-] LOGIN UNSUCCESSFULLY : WRONG PASSWORD  \n");
  }
  return;
}
//////////////////////////////////////////////////////////
bool groupexists(string groupid)
{
  for(auto x: groupdetails)
  {
    if(groupid == x.groupid)
    {
      return true;
    }
  }
  return false;
}

void creategroupfunction(vector<string> strippedcommands,User &u)
{
  string groupid=strippedcommands[1];
  if(groupexists(groupid))
  {
    //cout<<"[-] GROUP ALREADY EXISTS "<<endl;
    clientreply(u,"[-] GROUP ALREADY EXISTS  \n");
    return;
  }

  if(logindetails[u.userid]==false)
  {
    //cout<<"[-] PLEASE LOGIN FIRST "<<endl;
    clientreply(u,"[-] PLEASE LOGIN FIRST  \n");
    return;
  }


  Group newgroup;
  newgroup.groupid=groupid;
  newgroup.users.push_back(u.userid);
  groupdetails.push_back(newgroup);
  //cout<<"[+] GROUP CREATED : "<<groupid<<endl;
  clientreply(u,"[+] GROUP CREATED:"+groupid+"\n");
  return;

}
/////////////////////////////////////////////////////////////////
void joingroupfunction(vector<string> strippedcommands,User &u)
{
  string groupid=strippedcommands[1];
  if(!groupexists(groupid)||u.userid=="")
  {
    clientreply(u,"[-] GROUP DOES NOT EXISTS \n");
    //cout<<"[-] GROUP DOES NOT EXISTS "<<endl;
    return;
  }
  joingroup(groupid,u);
}

void joingroup(string groupid,User &u)
{
    for(auto &x: groupdetails)
    {
      if(x.groupid == groupid)
      {
          if(existsingroup(x,u.userid))
          {
           // cout<<"[-] REQUEST TO JOIN ALREADY EXISTS "<<endl;
            clientreply(u,"[-] REQUEST TO JOIN ALREADY EXISTS \n");
            return;
          }

          x.pendingrequests.push_back(u.userid);
          // cout<<"[+] REQUESTED TO JOIN "<<groupid<<endl;  
           clientreply(u,"[+] REQUESTED TO JOIN "+groupid+"\n");    
      }
    }
}

bool existsingroup(Group grp,string userid)
{
  for(auto x:grp.pendingrequests)
  {
    if(x == userid)
    {
      return true;
    }
  }
  return false;
}

bool partofgroup(Group grp,string userid)
{
  for(auto x:grp.users)
  {
    if(x == userid)
    {
      return true;
    }
  }
  return false;
}
////////////////////////////////////////////////////////////////////
void leavegroupfunction(vector<string> strippedcommands,User &u)
{
  string groupid=strippedcommands[1];
  if(!groupexists(groupid)||u.userid=="")
  {
    //cout<<"[-] GROUP DOES NOT EXISTS "<<endl;
    clientreply(u,"[-] GROUP DOES NOT EXISTS \n");
  }
  leavegroup(groupid,u);
}

void leavegroup(string groupid,User &u)
{
  for(auto x: groupdetails)
    {
      if(x.groupid == groupid)
      {
          if(existsingroup(x,u.userid))
          {
            leavefromgroup(x,u);

          }
          //cout<<"[-] CANNOT LEAVE GROUP: NOT A PART OF GROUP "<<endl;    
          clientreply(u,"[-] CANNOT LEAVE GROUP: NOT A PART OF GROUP \n");
      }
    }

}
void leavefromgroup(Group &grp,User &u)
{

  auto it = find (grp.users.begin(), grp.users.end(), u.userid);
  grp.users.erase(it);
  //cout<<"[+] "<<u.userid<<" LEFT THE GROUP "<<grp.groupid<<endl;
  clientreply(u,"[+] "+u.userid+" LEFT THE GROUP "+grp.groupid+"\n");
}
//////////////////////////////////////////////////////////////////

void listpendingrequestsfunction(vector<string> strippedcommands,User &u)
{
    string groupid=strippedcommands[2];
    if(!groupexists(groupid))
    {
      clientreply(u,"[-] GROUP DOESN'T EXIST \n");
      return;
    }
    listpendingrequests(groupid,u);
    return;
}

void listpendingrequests(string groupid,User &u)
{
  string rep="[+] DETAILS OF "+groupid+" ARE LISTED AS : \n";

    for(auto &x:groupdetails)
    {
      if(x.groupid == groupid)
      {
        for(auto y: x.pendingrequests)
        {
          rep=rep+y+"\n";
        }
      }
    }
    clientreply(u,rep);
}
//////////////////////////// ACCEPT REQUEST /////////////
void acceptrequestfunction(vector<string> strippedcommands,User &u)
{
  
  string groupid=strippedcommands[1];
  string userid=strippedcommands[2];
  if(groupexists(groupid)&&userexists(userid))
  {
    accept_request(groupid,userid,u);
    return;
  }
  //cout<<"[-] GROUP/USER DOESN'T EXIST "<<endl;
  clientreply(u,"[-] GROUP/USER DOESN'T EXIST \n");
  return;
}

void accept_request(string groupid,string userid,User &u)
{
  for(auto &x:groupdetails)
  {
    if(x.groupid == groupid && existsingroup(x,userid) && u.userid == x.users[0])
    {
        x.users.push_back(userid);
        //cout<<"[+] USER "<<userid<<" ADDED TO GROUP "<<endl;
        clientreply(u,"[+] USER "+userid+" ADDED TO GROUP "+"\n");
        removefromlist(groupid,userid);
        return;
    }
  }
}

void removefromlist(string groupid,string userid)
{
  for(auto grp:groupdetails)
  {
    if(grp.groupid == groupid)
    {
      auto it = find (grp.pendingrequests.begin(), grp.pendingrequests.end(), userid);
      grp.pendingrequests.erase(it);
    }
  }
}
/////////////////////////////// LIST ALL GROUPS  //////////////

void listallgroupsfunction(User &u)
{
  string rep="[+] LISTING ALL GROUPS \n";
  for(auto &x:groupdetails)
  {
    rep=rep+x.groupid+"\n";
  }
  clientreply(u,rep);
}
///////////////////////////////  LOGOUT   ///////////////////////

void logoutfunction(User &u)
{
  if(u.userid=="")
  {
    clientreply(u,"[-] ALREADY LOGGED OUT \n");
    //cout<<"[-] ALREADY LOGGED OUT"<<endl;
    return;
  }

  if(!logindetails[u.userid])
  {
    clientreply(u,"[-] ALREADY LOGGED OUT \n");
    return;
  }
  logindetails[u.userid]=false;
  u.userid="";
  clientreply(u,"[+] SUCCESSFULL LOGOUT  \n");
  //cout<<"[+] SUCCESSFULL LOGOUT "<<endl;
}

//////////////////////////////////////////////////////////////////
string stripfilename(string filepath)
{
  string strippedname="";
  for(int i=filepath.length()-1;i>=0;i--)
  {
    if(filepath[i]!='/')
    {
      strippedname.push_back(filepath[i]);
    }
    else{
      break;
    }
  }
  reverse(strippedname.begin(),strippedname.end());
  return strippedname;

}

void uploadfunction(vector<string> strippedcommands,User &u)
{
  string filepath=strippedcommands[1];
  string filename=stripfilename(filepath);
  string groupid=strippedcommands[2];
  string filesize=strippedcommands[3];
  string filesha1=strippedcommands[4];
  if(!groupexists(groupid))
  {
    clientreply(u,"[-] GROUP DOES NOT EXIST \n");
    return;
  }
  for(auto &x: groupdetails)
  {
    if(x.groupid==groupid)
    {
      uploadfile(x,filepath,filename,filesize,filesha1,u);
    }
  }
}
//////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
bool existsingfile(Group &grp,string filename)
{
    for(auto x: grp.chunkdetails)
    {
      if(x.filename==filename)
      {
        cout<<"true"<<endl;
        return true;

      }
    }
    cout<<"false"<<endl;
    return false;
}

/////////////////////////////////////////////////////////////
void uploadfile(Group &grp,string filepath,string filename,string filesize,string fileSHA1,User &u)
{

  grp.listoffiles.push_back(filename);
  grp.filepathdetails.insert({filename,filepath});
  grp.filedetails.insert({filename,u.userid});
  
  grp.fileSHAdetails.insert({filename,fileSHA1});
  cout<<"[+] FILE UPLOADED "<<filename<<" :: SHA :: "<<fileSHA1<<endl;
  //..............................................//
  if(!existsingfile(grp,filename))
  {
    TorrentFile seed;
    seed.filename=filename;
    seed.filesize=stoi(filesize);
    int chunk;
    if(seed.filesize%(512*1024)==0)
    {
      chunk=(seed.filesize/(512*1024));
    }
    else{
      chunk=(seed.filesize/(512*1024))+1;
    }
    
    cout<<"[T]"<<chunk<<" "<<seed.filesize<<endl;
    for(int i=0;i<chunk;i++)
    {
      seed.partsoffile.insert({i,u.userid});
    }
    grp.chunkdetails.push_back(seed);
  }
  else{
     for(auto &x:grp.chunkdetails)
     {
        if(x.filename==filename)
        {
          int chunk=ceil(stoi(filesize)/(512*1024));
          cout<<chunk<<"\n";
          for(int i=0;i<chunk;i++)
          {
           x.partsoffile.insert({i,u.userid});
          }
          //grp.chunkdetails.insert({filename,seed});
        }

     }
  }
  //................................................//
  clientreply(u,"[+]"+filepath+" UPLOAD SUCCESSFULL\n");

}
/////////////////////////////////////////////////////////////////

void listfilesfunction(vector<string> strippedcommands,User &u)
{
  string groupid=strippedcommands[1];
  for(auto &x:groupdetails)
  {
    if(x.groupid==groupid&&existsingroup(x,u.userid))
    {
      string reply="";
      for(auto y: x.listoffiles)
      {
        reply =reply+y+"\n";
      }
      clientreply(u,"[+]LISTING ALL FILES \n"+reply);
      return;
    }
  }
  clientreply(u,"[-]LISTING UNSUCCESSFULL \n");
}
///////////////////////////////////////////////////////////////////

void downloadfilesfunction(vector<string> strippedcommands,User &u)
{
  string groupid=strippedcommands[1];
  string filename=strippedcommands[2];
  string destinationpath=strippedcommands[3];
  for(auto &x: groupdetails)
  {
    if(x.groupid==groupid)
    {
      downloadfiles(x,filename,destinationpath,u);
      return;
    }
  }
  clientreply(u,"[-] UNSUCCESSFULL DOWNLOAD\n");
}

int randomindexer(int sizeofvector)
{
  srand(time(0));
  return rand()%sizeofvector;
 
}
void downloadfiles(Group &grp,string filename,string destinationpath,User &u)
{
  if(!partofgroup(grp,u.userid))
  {
    clientreply(u,"[-]USER NOT A PART OF GROUP \n");
    return;
  }
  for(auto &x: grp.chunkdetails)
  {
    cout<<"[]"<<x.filename<<endl;
    if(x.filename==filename)
    {
      ////////////////////////////////////
      string reply="";
      //////////////////////////////////
      int filesize=x.filesize;
       int chunks;
       if(x.filesize%(512*1024)==0)
        {
          chunks=(x.filesize/(512*1024));
        }
        else{
          chunks=(x.filesize/(512*1024))+1;
        }
    
      reply=to_string(chunks)+" "+to_string(filesize);
      for(int i=0;i<chunks;i++)
      {
        auto it=x.partsoffile.find(i);
        if(it ==x.partsoffile.end())
        {
        break;
        }

        auto it1 = x.partsoffile.equal_range(i);
        vector<string> userids;
        int sizeofvector=0;
        for (auto itr = it1.first; itr != it1.second; ++itr)
        {
          userids.push_back(itr->second);
          sizeofvector++;
        }
        string userid=userids[randomindexer(sizeofvector)];
        int peerport=portdetails[userid];
        string peer_port=to_string(peerport);
        reply=reply+" "+peer_port;
      }
      
      /////////////////////////////////////
      
      // sending port of userid
      
      //cout<<"[*] TESTING "<<peer_port<<endl;
      //clientreply(u,peer_port);
      /////////////////////////////////////
      
      cout<<reply<<endl;
      clientreply(u,reply);

      //grp.filepathdetails.insert({filename,destinationpath+"/"+filename});
      //grp.filedetails.insert({filename,u.userid});


      return;
    }
  }
  clientreply(u,"[-]DOWNLOAD UNSUCCESSFULL \n");
}
//////////////////////////////////////////////////////////////////////////

void updatechunkfunction(vector<string> strippedcommands,User &u)
{
  string userid=u.userid;
  string filename=strippedcommands[1];
  string chunknum=strippedcommands[2];
  string groupid=strippedcommands[3];
  int chunknumber=stoi(chunknum);
  //.....................................///
  for(auto &x: groupdetails)
  {
    if(x.groupid==groupid)
    {
      for(auto &y:x.chunkdetails)
     {
        if(y.filename==filename)
        {
                    
          y.partsoffile.insert({chunknumber,u.userid});
          clientreply(u,"[+] CHUNK UPDATED: "+filename+":"+chunknum+" ::"+userid+" "+chunknum+"\n");
          //grp.chunkdetails.insert({filename,seed});
          return;
        }

     }
    }
  }
    clientreply(u,"[+] CHUNK UPDATE FAILED: \n");

}

void stopsharingfunction(vector<string> strippedcommands,User &u)
{
  string groupid=strippedcommands[1];
  string filename=strippedcommands[2];
  for(auto &x: groupdetails)
  {
    if(x.groupid==groupid)
    {
      for(auto y=x.listoffiles.begin();y!=x.listoffiles.end();y++)
      {
        if(*y==filename)
        {
          x.listoffiles.erase(y);
          clientreply(u,"[-] STOP SHARING SUCCESSFULL: \n");
          return;
        }
      }

    }
  }
  clientreply(u,"[-] STOP SHARING FAILED \n");

}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
};
void readtoquit()
{
  while(1)
  {
    string ifquit;
    cin>>ifquit;
    if(ifquit=="quit")
    {
      exit(0);
    }
  }
}

int main(int argc, char** argv){
///................................////

string path=argv[2];
    fstream serverfile(path,ios::in);
    vector<string> IP_PORT_TRACKER;
    string temp;
    while(getline(serverfile,temp,' ')){
        IP_PORT_TRACKER.push_back(temp);
    }
    //string ip=IP_PORT_TRACKER[0];
    string server_port=IP_PORT_TRACKER[1];
////............................////

Server myserver(stoi(server_port));
myserver.serverbind();
myserver.serverlisten();
thread t1(readtoquit);
t1.detach();
//myserver.acceptconnection();
while(1)
{
  myserver.acceptconnection();
  
}

  return 0;
}