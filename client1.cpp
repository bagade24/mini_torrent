#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<iostream>
#include<thread>
#include<sys/stat.h>
#include<sys/types.h>
#include<bits/stdc++.h>
#include<openssl/sha.h>
#define SIZE 512
using namespace std;
///////////////////////////////////////////////////////////

/////    g++ client1.cpp -o client1 -lssl -lcrypto -pthread 


string convertToString(char* a, int size)
{
    string s = a;
    return s;
}


 map<string,string> myfiles;
 map<string,string>filestatus;
//////////////////////////////////////////////////////////////////////////////////////
class Socket{
public:
  char ip[10] = "127.0.0.1";
    int port=8080;
    int e;
    int sockfd;
    struct sockaddr_in server_addr;
    FILE *fp;
    string filename;
    int opt = 1;
    int new_sock=-1;
    socklen_t addr_size;
    struct sockaddr_in new_addr;
    string srcfile;
    string currentgroup="";

    Socket(int portnum)
    {

      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if(sockfd < 0) {
       perror("[-]Error in socket");
      exit(1);
      }
      printf("[+]Server socket created successfully. %d\n",portnum);
      this->port=portnum;
      setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt));
      server_addr.sin_family = AF_INET;
      server_addr.sin_port = portnum;
      server_addr.sin_addr.s_addr = inet_addr(ip);
      srcfile=to_string(portnum);
      mkdir(srcfile.c_str(), 0777);
      
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
      void serveraccept()
      {
        addr_size = sizeof(new_addr);
        new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);
        if(new_sock<0)
        {
           perror("[-]Error in ACCEPTANCE\n");
           exit(EXIT_FAILURE);
        }
        else{
          cout<<"[+] Acceptance "<<new_sock<<endl;
        }
      }



    bool connecttoserver(int portnum)
    {
      e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
      if(e == -1) {
      perror("[-]Error in socket");
      exit(1);
      }
      printf("[+]Connected to Server.\n");
      string reply=to_string(portnum);
      write(sockfd, reply.c_str(), reply.size());
    }

    bool connecttopeer()
    {
      e = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
      if(e == -1) {
      perror("[-]Error in socket");
      exit(1);
      }
      printf("[+]Connected to Another peer.\n");
      
    }

///////////////////////////////////////////////////////////////////////////////////////////////////
    void sendingfile(string filepath,int new_sock)
    {
      ifstream mySource;
      mySource.open(filepath, ios::in | ios::out | ios::trunc | ios::binary);
      
       int n;

       char databuf[1024]={0};
       char databuf1[1024]={0};
       int k=read(new_sock,databuf,1024);
       
      if (mySource.is_open())
      {
        while (!mySource.eof())
        {
            char data;
            mySource >> std::noskipws >> data;  
            //cout<<data;
            databuf[0]=data;
             if (send(new_sock, databuf, sizeof(databuf), 0) == -1) {
             perror("[-]Error in sending file.");
              cout<<"[-]Error"<<'\n';
             //exit(1);
           }
          bzero(databuf,1024);
          //cout<<"[]send"<<'\n';
          int n=read(new_sock,databuf,1024);
          if(databuf[0]=='O' && databuf[1]=='K'){
          bzero(databuf,1024);
            continue;
          }
        }
      }
      mySource.close();
  
    printf("[+]File data sent successfully.\n");

  } 
///////////////////////////////////////////////////////////////////////////////////////////////////    
    void sendfile(string filename,int new_sock)
    {
      //cotut<<
      fp = fopen(filename.c_str(), "r");
      if (fp == NULL)
      {
      perror("[-]Error in reading file.");
      exit(1);
      }

      send_file(fp, new_sock);
      printf("[+]File data sent successfully.\n");
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////
    void sendcommand(string command)
    {

      char buffer[1024] = {0};
      //cout<<" Here "<<endl;
      //write(sockfd, command.c_str(), command.size());
      if(stripcommand(command)==1) return;
      //cout<<"[T]\n";
      send(sockfd , command.c_str() , command.size() , 0 );
      char buffer1[SIZE]={0};
      int valread = read(sockfd ,buffer1, SIZE);
      int buffer_size = sizeof(buffer1) / sizeof(char);
      string response=convertToString(buffer1,buffer_size);

      cout<<response;
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////
int stripcommand(string command)
{
  string mycommand=command;
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
  if(strippedcommands[0]=="download_file")
  {
      char buffer[1024] = {0};
        if(strippedcommands[3]!=".") 
        {
          srcfile=strippedcommands[3];
        }
      string path=strippedcommands[2];
      currentgroup=strippedcommands[1];
      send(sockfd , command.c_str() , command.size() , 0 );
      char buffer1[SIZE]={0};
      int valread = read(sockfd ,buffer1, SIZE);
      int buffer_size = sizeof(buffer1) / sizeof(char);
      string reply=convertToString(buffer1,buffer_size)+" ";
      //cout<<reply<<endl;
      cout<<"[T]"<<path<<"\n";
      if(reply=="[-]DOWNLOAD UNSUCCESSFULL") return 1;
      //////////////////////////////////////////////////
      vector<string> strippedresponse;
     string str="";
     reply+=" ";
     for(int i=0;i<reply.length();i++)
      {
        if(reply[i] != ' ')
        {
          str+=reply[i];
        }
        else{
        strippedresponse.push_back(str);
         str="";
        }  
      } 
      ///////////////////////////////////////////////////
      filestatus[strippedcommands[2]]="DOWNLOADING";
      int chunk_size=stoi(strippedresponse[0]);
      int filesize=stoi(strippedresponse[1]);
      for(int i=0;i<chunk_size;i++)
      {
        thread t1(&Socket::downloadfile,this,path,stoi(strippedresponse[2+i]),filesize,i,chunk_size); 
        t1.detach();
        string updatechunk="update_chunk "+strippedcommands[2]+" "+to_string(i)+" "+strippedcommands[1];

        send(sockfd , updatechunk.c_str() , updatechunk.size() , 0 );
        char buffer1[SIZE]={0};
        int valread = read(sockfd ,buffer1, SIZE);
        int buffer_size = sizeof(buffer1) / sizeof(char);
        string reply=convertToString(buffer1,buffer_size)+" ";
        cout<<reply;
        
      }

      myfiles[filename]=srcfile+"/"+filename;
      filestatus[filename]="COMPLETE";
      cout<<"[+] FILE WRITTEN\n";
      /*int portid=stoi(strippedresponse[0]);
      string path=strippedresponse[1];
      int filesize=stoi(strippedresponse[2]);
      ////////////////////////////////////////////   
      /*char buffer2[1024] = {0};
      valread = read(sockfd ,buffer2, SIZE);
      buffer_size = sizeof(buffer2) / sizeof(char);
      string path=convertToString(buffer2,buffer_size);
                                                */
      /*int clientcon=connecttoclient(portid);
      command="downloadfile";
      send(clientcon, command.c_str() , command.size() , 0 );
      send(clientcon,path.c_str(),path.size(),0);
      writefile(clientcon);
      thread t1(&Socket::downloadfile,this,path,portid,filesize);
      //downloadclient(path,portid);  
      t1.detach();
      //downloadfile(path,portid);*/
      return 1;    
  }

  if(strippedcommands[0]=="upload_file")
  {
      char buffer[1024] = {0};
      string filename=stripfilepath(strippedcommands[1]);
      //cout<<"[] "<<filename<<" --> "<<strippedcommands[1]<<endl;
      string filesha1=getSHA(strippedcommands[1]);
      myfiles[filename]=strippedcommands[1];
      long long filesize=getfilesize(strippedcommands[1]);
      command=command+" "+to_string(filesize)+" "+filesha1;
      send(sockfd , command.c_str() , command.size() , 0 );
      char buffer1[SIZE]={0};
      int valread = read(sockfd ,buffer1, SIZE);
      int buffer_size = sizeof(buffer1) / sizeof(char);
      string reply=convertToString(buffer1,buffer_size)+" ";
      cout<<reply;
      filestatus[filename]="COMPLETE";

      return 1;    
  }

  if(strippedcommands[0]=="show_downloads")
  {
     for(auto x: filestatus)
     {
      cout<<"[+] "<<x.first<<" ["<<x.second<<"]"<<endl;
     }

      return 1;    
  }

  return 0;
}
/////////////////////////////////////////////////.........../////////////////////////////
string stripfilepath(string filepath)
{
  string filename="";
  int filesize=filepath.size();
  for(int i=filesize-1;i>=0;i--)
  {
    if(filepath[i]=='/')
    {
      break;
    }
    filename+=filepath[i];
  }
  reverse(filename.begin(),filename.end());
  return filename;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void downloadfile(string path,int portid,int filesize,int chunknum,int chunks)
{
  Socket *downloader= new Socket(portid);
  downloader->connecttopeer();
  int clientcon=downloader->sockfd;
  string command="downloadfile "+to_string(chunknum);
  int res1=-1,res2=-1;
  //cout<<"[+] download file "<<clientcon<<endl;
  while(res1==-1&&res2==-1)
  {
  res1=send(clientcon, command.c_str() , command.size(),0);
  char buffer[SIZE]={0};
  int valread = read(clientcon,buffer,sizeof(buffer));
  res2=send(clientcon,path.c_str(),path.size(),0);
  valread = read(clientcon,buffer,sizeof(buffer));
  //cout<<"[] send status 1 "<<res1<<endl;
  //<<"[] send status 2 "<<res2<<endl;
  }
  
  //writefile(clientcon,path);
  writingfile2(clientcon,path,filesize,chunknum,chunks);
  delete(downloader);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
void writefile(int new_sock,string path)
{

  write_file(new_sock,path);
  printf("[+]Data written in the file successfully.\n");
}

void write_file(int sockfd,string path)
{
  int n;
  FILE *fp;
  char filename[] = "recv.txt";
  char buffer[SIZE];
  string command="ok";
  send(new_sock, command.c_str() , command.size(),0);

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
/////////////////////////////////////////////////////////////////////////////////////////////////////
void writingfile(int clientcon,string path)
{
 int n;
  FILE *fp;
  ofstream myOutpue;
  //cout<<"[+] IN WRITING FILE\n";

  string par=srcfile+"/";
   myOutpue.open(par+path, ios::in | ios::out | ios::trunc | ios::binary);

  //fp = fopen(filename, "wb+");
  char databuf[1024]={0};
  char databuf1[1024]={0};
       
       char ack[]="ack";
       send(clientcon, ack, sizeof(ack), 0);
       

  while (1) {
  
    n = recv(clientcon, databuf, SIZE, 0);
    //cout<<n<<" "<<databuf<<endl;
    if (n <= 0){
      break;
      return;
    }

    myOutpue << databuf[0];
    //cout<<"recieve"<<'\n';
    bzero(databuf,1024);
    databuf[0]='O';
    databuf[1]='K';
    send(clientcon,databuf,1024,0);
    bzero(databuf,1024);
    
  }
   myOutpue.close(); 
  return;
}
///////////////////////////////////////////////////////////////////////////////////////////////

long getfilesize(string filename)
{
    FILE *fp = fopen(filename.c_str(), "r+");
    if (fp == NULL)
    {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long long file_size = ftell(fp);

    return file_size;
}
////////////////////////////////////////////////////////////////////////////////////////////////
void die(const char *s) {
    perror(s);
    exit(1);
}

void sendingfile2(string filepath,int new_sock,int chunknum)
{
  //cout<<"[+] SENDING started \n";

  
  char buff[256];
    bzero(buff, 256);
    string path=filepath;
    //cout<<"[+]"<<path<<endl;
    long long total_size = getfilesize(path);
    int chunk_size = 512*1024;
  
    //cout<<"[C] "<<chunk_size<<" "<<total_size<<"\n";
    int total_chunks = total_size / chunk_size -1;
    
    /*cout << buff << endl;
    ifstream source_file(path, ifstream::binary );
    char * buffer = (char *) malloc(chunk_size + 1);
    long long pos;
    for (pos = chunk_size*chunknum;pos < (total_size - (total_size % chunk_size)); pos += chunk_size) {
        int idx = 0;
        source_file.seekg(pos,ios::beg);
        source_file.read(buffer,chunk_size);
        write(new_sock, buffer, chunk_size);
        cout<<"[1]"<<buffer<<endl;
        bzero(buff,256);
        if (read(new_sock, buff, 256) <= 0)
            die("ERROR reading from socket");
        if (strcmp(buff,"OK") != 0){
            break;
        }
        bzero(buffer,chunk_size);
    }
    source_file.seekg(pos,ios::beg);
    source_file.read(buffer,total_size % chunk_size);
     cout<<"[2]"<<buffer<<endl;
    write(new_sock, buffer, total_size % chunk_size);
    bzero(buff,256);
    /*if (read(new_sock, buff, chunk_size) <= 0)
        die("ERROR reading from socket");
    */
    
    //source_file.close();
    long long pos=chunknum*chunk_size;
    FILE *fp = fopen(filepath.c_str(), "r+");
    fseek(fp, pos, SEEK_SET);
    char * buffer = (char *) malloc(total_size + 1);
        // sending file to client by reading in chunks
        int bytesReadFromFile;

        bytesReadFromFile = fread(buffer, sizeof(char), total_size, fp);
        send(new_sock, buffer, bytesReadFromFile, 0);
        bzero(buffer, sizeof(buff));
        //cout<<".............."<<pos<<endl;
        //printf("File : %s ,Piece = %d , %d Bytes sent!\n", filename.c_str(), pieceNo, bytesReadFromFile);
    fclose(fp);

   //cout<<"[+] SENDING DONE \n";
}

void writingfile2(int new_sock,string path,int filesize,int chunknum,int chunks)
{
  //cout<<"[+] WRITING STARTED \n";
  int chunk_size = 512*1024 , byte_read, prev_read=0;

    char * buff = new char[chunk_size];
    char buffer[chunk_size];
    long long total_size = filesize  , chunk_no, pos=chunknum*chunk_size;
    path = srcfile+"/"+path;
    cout<<"[+]"<<path<<endl;
    //cout<<"[+]"<<path<<endl;
    bzero(buffer,chunk_size);
    
    if(chunknum==chunks-1)
    {
      chunk_size=filesize-((filesize/chunk_size)*chunk_size);
    }
    
    //cout<<"[T] "<<chunk_size<<" "<<pos<<"\n";
   /* ofstream destination_file(path, ios_base::out | ios_base::binary | ios_base::trunc );
    for (pos = chunknum*chunk_size;pos < (total_size - (total_size % chunk_size)); pos += chunk_size) {
        destination_file.seekp(pos,ios::beg);
        while(prev_read!=chunk_size){
            bzero(buff,chunk_size);
            byte_read = read(new_sock, buff, chunk_size);
            prev_read += byte_read;

            destination_file.write(buff,byte_read);
        }
        prev_read=0;
        write(new_sock,"OK",3);
    }
    while(prev_read!=(total_size%chunk_size)) {
        bzero(buff,chunk_size);
        byte_read = read(new_sock, buff, chunk_size);
        prev_read += byte_read;
        destination_file.write(buff,byte_read);
    }
    */
     FILE *tmp = fopen(path.c_str(), "a");
    fclose(tmp);
    FILE *fd = fopen(path.c_str(), "r+");

    fseek(fd, pos, SEEK_SET);
        while (prev_read < chunk_size)
        {
            byte_read = recv(new_sock, buffer, chunk_size-prev_read, 0);
            //cout << "bytes read =" << byte_read << endl;
            prev_read += byte_read;
            fwrite(buffer, sizeof(char), byte_read, fd);
            // cout<<readBuffer<<endl;
            bzero(buffer, sizeof(buffer));
        }
        fclose(fd);
    //destination_file.close();
   // cout<<"[+] WRITING DONE \n";
   
}
/////////////////////////////////////////////////////////////////////////////////////////////////////    
    void closeconnection()
    {
      printf("[+]Closing the connection.\n");
      close(sockfd);
    }

    void send_file(FILE *fp, int sockfd){
    int n;
    char data[SIZE] = {0};
  
   char buffer[SIZE]={0};
   bzero(buffer, SIZE);
   int valread = read( new_sock , buffer, 1024);
   //cout<<"[] "<<buffer<<endl;
    while(fgets(data, SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, SIZE);
      }
  }
  void acceptconnection()
  {
     addr_size = sizeof(server_addr);
     new_sock = accept(sockfd, (struct sockaddr*)&server_addr, &addr_size);

     cout<<"[+] Peer accepted "<<new_sock<<sockfd<<endl;
     if(new_sock>=0)
     {
      char buffer[SIZE]={0};
      int valread = read( new_sock , buffer, 1024);
      int buffer_size = sizeof(buffer) / sizeof(char);
      string mycommand=convertToString(buffer,buffer_size);
      mycommand+=" ";
      ///----------------------------------------------------///////
      vector<string> strippedresponse;
      string str="";
     
      for(int i=0;i<mycommand.length();i++)
      {
        if(mycommand[i] != ' ')
        {
          str+=mycommand[i];
        }
        else{
        strippedresponse.push_back(str);
         str="";
        }  
      } 
      /////.................................................//////////
      if(strippedresponse[0]=="downloadfile")
      {
        char buffer1[SIZE]={0};
        int valread = read( new_sock , buffer1, 1024);
        int buffer_size = sizeof(buffer1) / sizeof(char);
        string filepath=convertToString(buffer1,buffer_size);
        //cout<<"[+]"<<filepath<<"\n";
        int chunk=0;
        thread t1(&Socket::sendingfile2,this,filepath,new_sock,chunk);
      }
     }
  }

  void uploadfile()
  {
    //cout<<"[+] IN UPLOAD FILE sock value: "<<new_sock<<endl;
    string mycommand="";
    while(mycommand=="")
    {
      char buffer[SIZE]={0};
      int valread = read(new_sock,buffer,1024);
      //cout<<valread<<endl;
      //string ack="justwastingtime";
      //send(new_sock , ack.c_str() , ack.size() , 0 );
      int buffer_size = sizeof(buffer) / sizeof(char);
      mycommand=convertToString(buffer,buffer_size);
      //cout<<"[+] COMMAND FROM PEER "<<mycommand<<endl;
      mycommand+=" ";
      ////---------------------------------------------------------///
      vector<string> strippedresponse;
      string str="";
     
      for(int i=0;i<mycommand.length();i++)
      {
        if(mycommand[i] != ' ')
        {
          str+=mycommand[i];
        }
        else{
        strippedresponse.push_back(str);
         str="";
        }  
      } 
      ////--------------------------------------------------------////
      if(mycommand!="")
      {
        string ack="justwastingtime";
        send(new_sock , ack.c_str() , ack.size() , 0 );

        char buffer1[SIZE]={0};
        int valread = read(new_sock,buffer1,1024);
        int buffer_size = sizeof(buffer1) / sizeof(char);
        string filepath=convertToString(buffer1,buffer_size);
        //cout<<"[+] filepath FROM PEER "<<filepath<<endl;
        filepath=myfiles[filepath];
         //cout<<"[+] filepath FROM SRC "<<filepath<<endl;
        int valread1 = send(new_sock , ack.c_str() , ack.size() , 0 );
        //cout<<buffer1<<endl;
        //thread t1(&Socket::sendfile,this,filepath,new_sock);
        //t1.detach();
        int chunks=stoi(strippedresponse[1]);
        sendingfile2(filepath,new_sock,chunks);
        //cout<<"[+] FILE UPLOADED\n";
        break;
      }
    }
    
  }
////////////////////////////////////////////////////////////////////////
string getSHA(string fpath)
{
    string reply="";
    cout << "fpath is " << fpath << endl;
   
    FILE *fp = fopen(fpath.c_str(), "r+");
    if (fp == NULL)
    {
        return "";
    }
    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    rewind(fp);

    unsigned char *buffer=(unsigned char*)malloc(sizeof(char) * SIZE);       
    unsigned char hash1[20];
    int n;
    n = fread(buffer, 1, sizeof(buffer), fp);
    SHA1(buffer, n, hash1);
    int i;
    string fileSHA;
    for (i = 0; i < 20; i++) {
        fileSHA.push_back(hash1[i]);
    }
   return fileSHA;
}
////////////////////////////////////////////////////////////////////
 
};


void clientconnection(Socket &myclient)
{
  cout<<"[+] Peer's client"<<endl;
  myclient.serverbind();
  myclient.serverlisten();
  while(1)
  {
     
    myclient.serveraccept();
    myclient.uploadfile();
    cout<<"[+]Acceptconnection Peer's client"<<endl;
    
  }
 myclient.closeconnection();
}


int main(int argc, char** argv)
{
  ///................................................///

    string path=argv[2];
    fstream serverfile(path,ios::in);
    vector<string> IP_PORT_TRACKER;
    string temp;
    while(getline(serverfile,temp,' ')){
        IP_PORT_TRACKER.push_back(temp);
    }
    //string ip=IP_PORT_TRACKER[0];
    string server_port=IP_PORT_TRACKER[1];
  //................................................////
  int port=stoi(argv[1]);
  Socket mysocket(stoi(server_port));
  Socket myclient(port);
  mysocket.connecttoserver(port);
  thread t1(clientconnection,std::ref(myclient));
  t1.detach();
  while(1)
  {
   
    string command="";
    std::getline(std::cin, command);
    //stripcommand(command);
    mysocket.sendcommand(command);

  }
  mysocket.closeconnection();
  myclient.closeconnection();

  return 0;
}