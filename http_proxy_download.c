/* f20171449@hyderabad.bits-pilani.ac.in Sai Satvik Vuppala */
/* This assignments is about the extraction of html text and image from the the http websites on the web using socket programming through a proxy server*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h> 
#include<arpa/inet.h> // function for converting network addresses

#define SIZE 1000

// The base64 enocder has been taken from the net, as I couldn't code the logic for it.
size_t Encoder_base_64d_size(size_t input_length)
{
	size_t lenre;

	lenre = input_length;
	if (input_length % 3 != 0)
		lenre += 3 - (input_length % 3);
	lenre /= 3;
	lenre *= 4;

	return lenre;
}

char* Encoder_base_64(const unsigned char *in, size_t len)
{
	//the range of accepted letters that are use for encoding
	const char range_b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char* encoded;
	size_t  elen;
	size_t  i;
	size_t  j;
	size_t  v;

	if (in == NULL || len == 0)
		return NULL;

	elen = Encoder_base_64d_size(len);
	encoded  = malloc(elen+1);
	encoded[elen] = '\0';

	for (i=0, j=0; i<len; i+=3, j+=4) {
		v = in[i];
		v = i+1 < len ? v << 8 | in[i+1] : v << 8;
		v = i+2 < len ? v << 8 | in[i+2] : v << 8;

		encoded[j]   = range_b64[(v >> 18) & 0x3F];
		encoded[j+1] = range_b64[(v >> 12) & 0x3F];
		if (i+1 < len){
			encoded[j+2] = range_b64[(v >> 6) & 0x3F];
		} else {
			encoded[j+2] = '=';
		}
		if (i+2 < len) {
			encoded[j+3] = range_b64[v & 0x3F];
		} else {
			encoded[j+3] = '=';
		}
	}
	//printf("%s",encoded);
	return encoded;
}


//gcc -o http_client http_client.c
//./http_client go.com 182.75.45.22 13128 csf303 csf303 index.html logo.gif
/*char* base64Encoder(char input_str[], int len_str) 
{ 
    char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";   
    char *res_str = (char *) malloc(SIZE * sizeof(char));      
    int index, no_of_bits = 0, padding = 0, val = 0, count = 0, temp; 
    int i, j, k = 0;
    for (i = 0; i < len_str; i += 3){     
            val = 0, count = 0, no_of_bits = 0;  
            for (j = i; j < len_str && j <= i + 2; j++){ 
                val = val << 8;  
                val = val | input_str[j];
                count++;              
            } 
  
            no_of_bits = count * 8;  
            padding = no_of_bits % 3; 
  			while (no_of_bits != 0)  
            { 
                if (no_of_bits >= 6) 
                { 
                    temp = no_of_bits - 6; 
                    index = (val >> temp) & 63;  
                    no_of_bits -= 6;          
                } 
                else
                { 
                    temp = 6 - no_of_bits; 
                    index = (val << temp) & 63;  
                    no_of_bits = 0; 
                } 
                res_str[k++] = char_set[index]; 
            } 
    } 
    
    for (i = 1; i <= padding; i++){ 
        res_str[k++] = '='; 
    }  
    res_str[k] = '\0';  
    return res_str; 
  
}
*/

void image_generation(int port_no,char* address,char res[],char* site,char* encoded,char* gif){
		int image_socket;
		image_socket= socket(AF_INET,SOCK_STREAM,0);
		
		//specifying an address to which the socket can connect
		struct sockaddr_in server_address;
		server_address.sin_family = AF_INET; //domain of the socket
		server_address.sin_port = htons(port_no); // The port number to which we want to send
		server_address.sin_addr.s_addr = inet_addr(address);
		//inet_aton(address,&server_address.sin_addr.s_addr); // IP Address of server
		
		
		//to establish connection 
		int cn_status_img = connect(image_socket, (struct sockaddr *) &server_address,sizeof(server_address));
		//checking for error in the connection 
		if(cn_status_img == -1){
			printf("There was an error in making the connection");
		}
		
		//"GET http://info.in2p3.fr/cc.gif HTTP/1.1\r\nHost: info.in2p3.fr\r\nAccept: */*\r\nConnection: close\r\nProxy-Authorization: Basic Y3NmMzAzOmNzZjMwMw==\r\n\r\n"
		/*char req_img[] = "GET http://";
		strcat(req_img,site);
		strcat(req_img,"/");
		strcat(req_img,res);*/
		//strcat(req_img," HTTP/1.1\r\nHost: info.in2p3.fr\r\nAccept: */*\r\nConnection: close\r\nProxy-Authorization: Basic ");
		//strcat(req_img,encoded);
		//strcat(req_img,"\r\n\r\n");		
		char url_img[] = "GET http://%s/%s HTTP/1.1\r\nHost: %s\r\nAccept: image/webp,*/*\r\nProxy-Connection: keep-alive\r\nConnection: close\r\nProxy-Authorization: Basic %s\r\n\r\n";
		//Connection: close\r\n
		char req_img[4096];
		sprintf(req_img,url_img,site,res,site,encoded);
			unsigned char img_serv_resp[32*1024];
			FILE* fimg;
			fimg = fopen(gif,"wb+");
			send(image_socket, req_img, sizeof(req_img), 0);
			char* img_str = "Connection: close\r\n\r\n";
			int rec_img;
			do{
				rec_img = recv(image_socket,img_serv_resp,32*1024,0);	
				char* img_str_1 = strstr(img_serv_resp,img_str);
				if(img_str_1 == NULL){
					//printf("%.*s",rec_img,img_serv_resp);
					//fprintf(fimg,"%.*s",rec_img,img_serv_resp);
					fwrite(img_serv_resp,rec_img,1,fimg);
				}else{
					//printf("%.*s",rec_img,img_serv_resp);
					//fprintf(fimg,"%.*s",rec_img,img_str_1+strlen(img_str));
					fwrite(img_str_1+strlen(img_str),rec_img-418,1,fimg);
				}
				//memset(img_serv_resp,0,sizeof(img_serv_resp));
			}while(rec_img > 0);
			
			close(image_socket);
			fclose(fimg);
}

void html_generator(int port_no,char* address,char* site,char* host,char* encoded,char* html,char* gif,int flag){
	//creating a socket 
	int network_socket;
	network_socket = socket(AF_INET,SOCK_STREAM,0);
	
	//specifying an address to which the socket can connect
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; //domain of the socket
	server_address.sin_port = htons(port_no); // The port number to which we want to send
	server_address.sin_addr.s_addr = inet_addr(address);
	//inet_aton(address,&server_address.sin_addr.s_addr); // IP Address of server
	
	
	//to establish connection 
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address,sizeof(server_address));
	//checking for error in the connection 
	if(connection_status == -1){
		printf("There was an error in making the connection");
	}
	
	//recieving data from the server
	//Y3NmMzAzOmNzZjMwMw==
	//"GET http://%s/ HTTP/1.1\r\nHost: %s\r\nProxy-Connection: keep-alive\r\nConnection: close\r\nAccept: */*\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8\r\nProxy-Authorization: Basic %s\r\n\r\n"
	//"GET http://go.com/ HTTP/1.1\r\nHost: go.com\r\nAccept: */*\r\nConnection: close\r\nProxy-Authorization: Basic Y3NmMzAzOmNzZjMwMw==\r\n\r\n"
	//Proxy-Connection: keep-alive\r\n
	//"GET http://%s/ HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nAccept: */*\r\nAccept-Encoding: gzip, deflate\r\nAccept-Language: en-GB,en-US;q=0.9,en;q=0.8\r\nProxy-Authorization: Basic %s\r\n\r\n"
	char request[1024];
	if(flag == 0){
		strcpy(request,"GET http://%s/ HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\nProxy-Authorization: Basic %s\r\n\r\n");
	}else if(flag == 1){
		strcpy(request,"GET http://%s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\nProxy-Authorization: Basic %s\r\n\r\n");
	}
		
	char req_msg[4096];
	sprintf(req_msg,request,site,host,encoded);
	char server_response[4096];
	//printf("%d\n",sizeof(server_response));
	FILE* fp;
	fp = fopen(html,"w+");
	//char arr[] = "";
	//printf("\n%s\n",req_msg);
	
	int send_html = send(network_socket, req_msg, sizeof(req_msg), 0);
	if(send_html < 0) printf("Send Failed");
	char* str = "Connection: close\r\n\r\n";
	int rec;
	
	do{
		rec = recv(network_socket,server_response,sizeof(server_response),0);
		char* string_loc = "Location: ";	
		char* str1_loc = strstr(server_response,string_loc);
		//if(str1_loc == NULL) printf("\nNULL\n"); 
		
		if(str1_loc != NULL){
			//printf("%s\n",str1_loc);
			char* site_loc = strstr(str1_loc,"://");
			
			site_loc = site_loc + 3;
			char res_host[1024];
			int j;
			//Location: http://www.google.com/
			for(j=0; *site_loc != '/';j++){
				res_host[j] = *site_loc;
				site_loc = site_loc + 1;
			}
			
			res_host[j] = '\0';
			char res_loc[1024];
			
			strcpy(res_loc,res_host);
			
			for(j=strlen(res_host); *site_loc != '\n';j++){
				res_loc[j] = *site_loc;
				site_loc = site_loc + 1;
			}
			
			res_loc[j-1] = '\0';
			
			//printf("%s\n%s\n",res_loc,res_host);
		
		close(network_socket);
		fclose(fp);
		//html_generator(int port_no,char* address,char* site,char* host,char* encoded,char* html,char* gif)
		html_generator(port_no,address,res_loc,res_host,encoded,html,gif,1);
		return;
		}
		
		char* str1 = strstr(server_response,str);
		if(str1 == NULL){
			//printf("%.*s",rec,server_response);
			fprintf(fp,"%.*s",rec,server_response);
		}else{
			//printf("%.*s",rec,str1+strlen(str));
			fprintf(fp,"%.*s",rec,str1+strlen(str));
		}

		memset(server_response,0,sizeof(server_response));
	}while(rec > 0);

	
	char imgstr[100] = "SRC=";
	char res[100];
	char* image_st;
	fseek(fp,0,SEEK_SET);
	if(!strcmp(site,"info.in2p3.fr")){
		char read[100];
		while (!feof(fp)) { 
  
		    fscanf(fp, "%s", read);
		    //printf("%s\n\n",read);
		    
		 	image_st = strstr(read,imgstr);   	  
		    if (image_st != NULL){
		    	//printf("Entered\n"); 
	  			image_st += 5;
	  			int i;
	  			for(i=0;*image_st != '\"' && i<strlen(read);i++){
	  				//if(i == 0) printf("Entered2\n");
	  				res[i] = *image_st;
	  				image_st += 1;
	  			}
	  			res[i+1] = '\0';	
	  			break; 
        	}
       }       
       //printf("%s\n\n",res);          
	}
	
	close(network_socket);
	
	if(!strcmp(site,"info.in2p3.fr")){
		image_generation(port_no,address,res,site,encoded,gif);	
	}
	
}


int main(int argc, char *argv[]){
	
	//checking if there are 8 parameters passed
	
	if(argc != 8){
		printf("Not the exact match of parameters needed, please try again");
		exit(1);
	}
	
	char* site;site = argv[1];
	
	char *address;address = argv[2];
	
	int port_no;port_no = atoi(argv[3]);
	
	char* username;username = argv[4];
	
	char* password;password = argv[5];
	
	char* html;html = argv[6];
	
	char* gif;gif = argv[7];
	
	int un_len = strlen(username);
	int pw_len = strlen(password);
	
	char encode[un_len+pw_len+1];
	
	/*for(int i=0;i<argc;i++){
		printf("%s\n",argv[i]);
	}
	
	printf("%d\n",port_no);
	*/
	//we need to pass it into encode in the format username:password
	// we need to copy the elements of username into encoded
	strcpy(encode,username);
	
	//we need to concatnate encode with :, password
	strcat(encode,":");
	strcat(encode,password);
	
	//printf("%s\n",encode);
	// length of encoded string
	int len_en = strlen(encode);
	
	char* encoded = Encoder_base_64(encode,len_en);	
	
	//printf("%s\n",encoded);
	html_generator(port_no,address,site,site,encoded,html,gif,0);	
	
	return 0;
}
