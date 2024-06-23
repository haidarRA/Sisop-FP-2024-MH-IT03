#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <libgen.h>
#include <crypt.h>
#include <arpa/inet.h>

#define SALT "$2a$10$KT2H8E35L3wNleZjpugbqO.Nc/TMOMEmABBs15Sxv7XleUTummLmi"
#define PORT 8086
#define IP "127.0.0.1"

int directory_exists(const char *path) {
    struct stat stats;

    stat(path, &stats);

    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}

char* get_path() {
    static char path[200];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return dirname(path);
    } else {
        perror("Error getting executable path");
        exit(EXIT_FAILURE);
    }
}

char* split_comma(char* str, int index) {
    char* token;
    char* rest = str;
    int count = 0;

    while ((token = strtok_r(rest, ",", &rest))) {
        count++;
        if (count == index) {
            return token;
        }
    }

    return NULL;
}

void remove_lastdir(char *path) {
    int length = strlen(path);
    if (length == 0 || (length == 1 && path[0] == '/')) {
        return;
    }

    int i;
    for (i = length - 1; i >= 0; i--) {
        if (path[i] == '/') {
            break;
        }
    }

    if (i < 0) {
        return;
    }

    if (i == length - 1) {
        path[i] = '\0';
    } 
    else {
        path[i] = '\0';
    }
}

int count_spaces(char *str) {
    int count = 0;
    for(int i = 0; str[i] != '\0'; i++) {
    	if(str[i] == ' ') {
      	    count++;
    	}
    }
    return count;
}

int main(int argc, char *argv[]) {
    char* u_path = get_path();

    char dcpath[256];
    sprintf(dcpath, "%s/DiscorIT", u_path);

    char userpath[256];
    sprintf(userpath, "%s/users.csv", dcpath);

    if(directory_exists(dcpath) == 0) {
	if(mkdir("DiscorIT", 0777) ==  -1) {
	    printf("Can't create directory for DiscorIT"); 
	}
	
	FILE *fuser = fopen(userpath, "a");
	
	char *broot = crypt("root", SALT);
	
	if (broot == NULL) {
	  perror("crypt");
	  return 1;
	}
	
	fprintf(fuser, "1,root,%s,ROOT\n", broot);
	
	fclose(fuser);

	char chpath[256];

	sprintf(chpath, "%s/channels.csv", dcpath);
	FILE *fch = fopen(chpath, "a");
	fclose(fch);
    }

    char line[1024];

    if(strcmp(argv[1], "REGISTER") == 0) {
    	FILE *fuser = fopen(userpath, "a+");
    	int id = 1;
    	int user_exists = 0;
    	while(fgets(line, sizeof(line), fuser)) {
    	    if(strstr(line, argv[2])) {
    	    	user_exists = 1;
    	    }
            id++;
    	}
	if(user_exists == 0) {
	    char *bpass = crypt(argv[4], SALT);
	    fprintf(fuser, "%d,%s,%s,USER\n", id, argv[2], bpass);
	    printf("%s berhasil register\n", argv[2]);
	}
	else if(user_exists == 1) {
	    printf("%s sudah terdaftar\n", argv[2]);
	}
    	fclose(fuser);
    }
    else if(strcmp(argv[1], "LOGIN") == 0) {
    	FILE *fuser = fopen(userpath, "a+");
    	char *bpass1, *bpass2, rank[20], check_line[256], check_rank[256];
    	int user_exists = 0;
    	while(fgets(line, sizeof(line), fuser)) {
    	    if(strstr(line, argv[2])) {
    	    	strcpy(check_line, line);
    	    	user_exists = 1;
    	    }
    	}
    	char *check_pass = argv[4];
    	check_pass[strcspn(check_pass, "\n")] = '\0';
    	bpass2 = crypt(check_pass, SALT);
    	bpass1 = split_comma(check_line, 3);

	fclose(fuser);
    	if(strcmp(bpass1, bpass2) == 0) {
            fuser = fopen(userpath, "a+");
    	    printf("%s berhasil login\n", argv[2]);
    	    while(fgets(line, sizeof(line), fuser)) {
    	    	if(strstr(line, argv[2])) {
    	    	    strcpy(check_rank, line);
	    	    sprintf(rank, "%s", split_comma(check_rank, 4));
    	        }
    	    }
   	    char input[125], result[100];
    	    char right[60], name[40];
    	    sprintf(name, "%s", argv[2]);
	    sprintf(right, "[%s]", argv[2]);
    	    int is_channel = 0, is_room = 0, channel_exists = 0, room_exists = 0;
    	    char channel[40], room[40], c1[30], c2[30];
    	    strcpy(channel, "");
    	    strcpy(room, "");
    	    fclose(fuser);
    	    while(true) {
		int sock = 0;
		struct sockaddr_in serv_addr;
    	    	printf("%s ", right);
    	    	fgets(input, 125, stdin);
    	    	input[strcspn(input, "\n")] = '\0';

		if(strcmp(input, "EXIT") == 0) {
		    if((is_channel == 1) && (is_room == 0)) { //saat di channel
		    	sprintf(right, "[%s]", name);
		    	is_channel = 0;
		    	strcpy(channel, "");
		    }
		    else if((is_channel == 1) && (is_room == 1)) {
		    	sprintf(right, "[%s/%s]", name, channel);
		    	is_room = 0;
		    	strcpy(room, "");
		    }
		    else if((is_channel == 0) && (is_room == 0)) { //tidak masuk di channel (masih di luar)
		    	//printf("di luar\n");
		    	break;
		    }
		}
		else if(strstr(input, "JOIN")) {
	    	    sscanf(input, "%s %s", c1, c2);
	    	    if(is_channel == 0 && is_room == 0) { //channel
	    	    	char check_channel[256];
			int channel_exists = 0;
			    	        
			char chpath[256];
			sprintf(chpath, "%s/channels.csv", dcpath);
	    	        
	    	        FILE *fch = fopen(chpath, "r+");
	    	        while(fgets(line, sizeof(line), fch)) {
	    	    	    if(strstr(line, c2)) {
	    	    	    	channel_exists  = 1;
				strcpy(check_channel, line);
				break;
	    	            }
	    	        }
	    	        fclose(fch);
	    	        
	    	        if(channel_exists = 1) { //proses masuk channel
			    char rankch[40];
			    char authpath[256], *chpass1, *chpass2;
			    int user_in_channel = 0;
			    sprintf(authpath, "%s/%s/admin/auth.csv", dcpath, c2);
			    FILE *fauth = fopen(authpath, "r+");
			    while(fgets(line, sizeof(line), fauth)) {
			        if(strstr(line, name)) {
				    user_in_channel = 1;
			    	    break;
			    	}
			    }
			    fclose(fauth);
			    
    			    rank[strcspn(rank, "\n")] = '\0';
			    if((user_in_channel == 0) && (strstr(rank, "USER"))) { //jika user belum ada auth dan bukan root
			    	char ch_pass[50];
			    	printf("Key: ");
		    	    	fgets(ch_pass, 50, stdin);
		    	    	ch_pass[strcspn(ch_pass, "\n")] = '\0';
			    	chpass2 = crypt(ch_pass, SALT);
			    	chpass1 = split_comma(check_channel, 3);
			    	int can_pass = strcmp(chpass1, chpass2);   
			    	if(can_pass == 10) {
	    	    	    	    //sprintf(right, "[%s/%s]", argv[2], c2);
			    	    int id = 1;
			    	    fauth = fopen(authpath, "a+");
			    	    while(fgets(line, sizeof(line), fauth)) {
				        id++;
			    	    }
			    	    fprintf(fauth, "%d,%s,USER\n", id, name);
			    	    fclose(fauth);
			    	    strcpy(channel, c2);
	    	    	    	    sprintf(right, "[%s/%s]", name, c2);
			    	    is_channel = 1;
			    	}
			    	else if(can_pass != 0) {
			    	    printf("Gagal masuk channel %s\n", c2);
			    	}
			    }
			    else if((user_in_channel == 1) || (strstr(rank, "ROOT"))) { //jika user sudah ada auth di channel atau merupakan root
			   	sprintf(right, "[%s/%s]", name, c2);
			    	strcpy(channel, c2);
			    	is_channel = 1;
			    }
	    	        }
	    	        else {
	    	            printf("Channel %s tidak ditemukan\n", c2);
	    	        }
	    	    }
	    	    else if(is_channel == 1 && is_room == 0) { //room
	    	    	sprintf(right, "[%s/%s/%s]", name, channel, c2);
	    	    	strcpy(room, c2);
	    	    	is_room = 1;
	    	    }
		}
		else {
		    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Socket creation failed");
			exit(EXIT_FAILURE);
		    }

		    serv_addr.sin_family = AF_INET;
		    serv_addr.sin_port = htons(PORT);

		    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
			perror("Invalid address/ Address not supported");
			exit(EXIT_FAILURE);
		    }

		    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			perror("Connection failed");
			exit(EXIT_FAILURE);
		    }
		    //printf("Sent: %s\n", channel);
		    if (send(sock, &input, sizeof(input), 0) < 0) {
		        perror("Send failed");
		        exit(EXIT_FAILURE);
		    }
		    if (send(sock, &rank, sizeof(rank), 0) < 0) {
		        perror("Send failed");
		        exit(EXIT_FAILURE);
		    }
		    if (send(sock, &name, sizeof(name), 0) < 0) {
		        perror("Send failed");
		        exit(EXIT_FAILURE);
		    }
		    if (send(sock, &channel, sizeof(channel), 0) < 0) {
		        perror("Send failed");
		        exit(EXIT_FAILURE);
		    }
		    
		    if (send(sock, &room, sizeof(room), 0) < 0) {
		        perror("Send failed");
		        exit(EXIT_FAILURE);
		    }
		    if (recv(sock, &result, sizeof(result), 0) < 0) {
		        perror("Receive failed");
		        exit(EXIT_FAILURE);
		    }
		    printf("%s\n", result);
		}
    	    }
    	}
    	else {
    	    printf("%s gagal login\n", argv[2]);
    	}
    }
    
    return 0;
}
