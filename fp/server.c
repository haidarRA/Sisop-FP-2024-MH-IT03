#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <libgen.h>

#define SALT "$2a$10$KT2H8E35L3wNleZjpugbqO.Nc/TMOMEmABBs15Sxv7XleUTummLmi"
#define PORT 8086
#define BUFFER_SIZE 1024

int count_spaces(char *str) {
    int count = 0;
    for(int i = 0; str[i] != '\0'; i++) {
    	if(str[i] == ' ') {
      	    count++;
    	}
    }
    return count;
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

int directory_exists(const char *path) {
    struct stat stats;

    stat(path, &stats);

    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
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

int remove_directory(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
      struct dirent *p;

      r = 0;
      while (!r && (p=readdir(d))) {
          int r2 = -1;
          char *buf;
          size_t len;

          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             continue;

          len = path_len + strlen(p->d_name) + 2; 
          buf = malloc(len);

          if (buf) {
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);
             if (!stat(buf, &statbuf)) {
                if (S_ISDIR(statbuf.st_mode))
                   r2 = remove_directory(buf);
                else
                   r2 = unlink(buf);
             }
             free(buf);
          }
          r = r2;
      }
      closedir(d);
   }

   if (!r)
      r = rmdir(path);

   return r;
}

void edit_channel(const char *filepath, const char *old_name, const char *new_name) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.temp", filepath);
    FILE *temp_file = fopen(temp_filepath, "w");
    if (!temp_file) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *pos = strstr(line, old_name);
        if (pos) {
            size_t old_name_len = strlen(old_name);
            size_t new_name_len = strlen(new_name);
            size_t pos_index = pos - line;

            fwrite(line, 1, pos_index, temp_file);

            fwrite(new_name, 1, new_name_len, temp_file);

            fwrite(pos + old_name_len, 1, strlen(pos + old_name_len), temp_file);
        } 
        else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (remove(filepath) != 0) {
        perror("Error deleting the original file");
        return;
    }
    if (rename(temp_filepath, filepath) != 0) {
        perror("Error renaming the temporary file");
        return;
    }
}

void delete_row(const char *filepath, const char *channel_name) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.temp", filepath);
    FILE *temp_file = fopen(temp_filepath, "w");
    if (!temp_file) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (!strstr(line, channel_name)) {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (remove(filepath) != 0) {
        perror("Error deleting the original file");
        return;
    }
    if (rename(temp_filepath, filepath) != 0) {
        perror("Error renaming the temporary file");
        return;
    }
}

void edit_username(const char *csvpath, const char *old_substring, const char *new_substring) {
    FILE *file = fopen(csvpath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.temp", csvpath);
    FILE *temp_file = fopen(temp_filepath, "w");
    if (!temp_file) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *token;
        char *rest = line;
        int count = 0;
        int token_index = 0;
        char modified_line[512];

        while ((token = strtok_r(rest, ",", &rest))) {
            count++;
            if (count == 2 && strcmp(token, old_substring) == 0) {
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", new_substring);
                token_index += strlen(new_substring) + 1;  // +1 for the comma
            } else {
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", token);
                token_index += strlen(token) + 1;  // +1 for the comma
            }
        }

        if (modified_line[token_index - 1] == ',') {
            modified_line[token_index - 1] = '\0';
        }

        fputs(modified_line, temp_file);
    }

    fclose(file);
    fclose(temp_file);

    if (remove(csvpath) != 0) {
        perror("Error deleting the original file");
        return;
    }
    if (rename(temp_filepath, csvpath) != 0) {
        perror("Error renaming the temporary file");
        return;
    }
}

void edit_password(const char *csvpath, const char *username, const char *new_pass) {
    FILE *file = fopen(csvpath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.temp", csvpath);
    FILE *temp_file = fopen(temp_filepath, "w");
    if (!temp_file) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *token;
        char *rest = line;
        int count = 0;
        int token_index = 0;
        char modified_line[512] = {0}; // Initialize the modified_line buffer
        char current_username[256] = {0}; // Buffer to store current username

        while ((token = strtok_r(rest, ",", &rest))) {
            count++;
            if (count == 2) { // Second field (username field)
                strncpy(current_username, token, sizeof(current_username) - 1);
            }
            if (count == 3 && strcmp(current_username, username) == 0) { // Third field (password field)
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", new_pass);
                token_index += strlen(new_pass) + 1;  // +1 for the comma
            } else {
                // Copy other fields as is
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", token);
                token_index += strlen(token) + 1;  // +1 for the comma
            }
        }

        if (modified_line[token_index - 1] == ',') {
            modified_line[token_index - 1] = '\0';
        }

        fputs(modified_line, temp_file);
    }

    fclose(file);
    fclose(temp_file);

    if (remove(csvpath) != 0) {
        perror("Error deleting the original file");
        return;
    }
    if (rename(temp_filepath, csvpath) != 0) {
        perror("Error renaming the temporary file");
        return;
    }
}



void reorder_ids(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.temp", filepath);
    FILE *temp_file = fopen(temp_filepath, "w");
    if (!temp_file) {
        perror("Error opening temporary file");
        fclose(file);
        return;
    }

    char line[512];
    int id = 1;
    while (fgets(line, sizeof(line), file)) {
        char *comma_pos = strchr(line, ',');
        if (comma_pos) {
            fprintf(temp_file, "%d%s", id, comma_pos);
            id++;
        }
    }

    fclose(file);
    fclose(temp_file);

    if (remove(filepath) != 0) {
        perror("Error deleting the original file");
        return;
    }
    if (rename(temp_filepath, filepath) != 0) {
        perror("Error renaming the temporary file");
        return;
    }
}


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char input[125], result[100], rank[40], name[40], channel[40], room[40];
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid, sid;        

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
      exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    while(1) {
        while(1) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            int bytes_read1 = read(new_socket, input, sizeof(input));
            if (bytes_read1 <= 0) {
                printf("Invalid data received from client\n");
                close(new_socket);
                continue;
            }
            
            int bytes_read2 = read(new_socket, rank, sizeof(rank));
            if (bytes_read2 <= 0) {
                printf("Invalid data received from client\n");
                close(new_socket);
                continue;
            }
            
            int bytes_read3 = read(new_socket, name, sizeof(name));
            if (bytes_read3 <= 0) {
                printf("Invalid data received from client\n");
                close(new_socket);
                continue;
            }
            
	    int bytes_read4 = read(new_socket, channel, sizeof(channel));
	    if (bytes_read4 <= 0) {
	        printf("Invalid data received from client\n");
	        close(new_socket);
	        continue;
	    }
      
/*      
            int bytes_read5 = read(new_socket, room, sizeof(room));
            if (bytes_read5 <= 0) {
                printf("Invalid data received from client\n");
                close(new_socket);
                continue;
            }
*/
	    int spaces = count_spaces(input);
//	    sprintf(result, "%d", spaces);
	    char* u_path = get_path();

            char dcpath[256];
	    sprintf(dcpath, "%s/DiscorIT", u_path);

	    char c1[20], c2[20], c3[20], c4[20], c5[20], line[256];
	    if(spaces == 1) {
	    	sscanf(input, "%s %s", c1, c2);
	    	if((strcmp(c1, "LIST") == 0) && (strcmp(c2, "CHANNEL") == 0)) {
		    char chpath[256];
		    sprintf(chpath, "%s/channels.csv", dcpath);

		    FILE *fch = fopen(chpath, "r");
		    if (!fch) {
			perror("Error opening channels file");
			return;
		    }

		    char line[256];
		    while (fgets(line, sizeof(line), fch)) {
			char *channel_name = split_comma(line, 2);
			strcat(result, channel_name);
			strcat(result, " ");
		    }
		    
		    if(strstr(result, "$y")) {
			char *removed = "$y";
			char *start = strstr(result, removed);
			char *end = start + strlen(removed);
			strcpy(start, end);
    		    }

		    fclose(fch);
	    	}
	    	else if((strcmp(c1, "LIST") == 0) && (strcmp(c2, "USER") == 0)) {
		    if(strstr(rank, "ROOT")) {
		        char userpath[256];
		        sprintf(userpath, "%s/users.csv", dcpath);

		        FILE *fuser = fopen(userpath, "r");
		        if (!fuser) {
			    perror("Error opening users file");
			    return;
		        }

		        char line[256];
		        while (fgets(line, sizeof(line), fuser)) {
			    char *user_name = split_comma(line, 2);
			    strcat(result, user_name);
			    strcat(result, " ");
		        }

		        if(strstr(result, "$y")) {
			    char *removed = "$y";
			    char *start = strstr(result, removed);
			    char *end = start + strlen(removed);
			    strcpy(start, end);
    		        }
		    	fclose(fuser);
    		    }
    		    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk melihat seluruh user");
    		    }
	    	}
	    	else if(strcmp(c1, "REMOVE") == 0) {
	    	    //sprintf(result, "%s %s", c1, c2);
	    	    
	    	    if(strstr(rank, "ROOT")) {
	    	    	char userpath[256];
	    	    	sprintf(userpath, "%s/users.csv", dcpath);
	    	    	
	    	    	int user_exists = 0;
	    	    	
		        FILE *fuser = fopen(userpath, "r");
	    	    	char line[256];
	    	    	while(fgets(line, sizeof(line), fuser)) {
	    	    	    if(strstr(line, c2)) {
	    	    	    	user_exists = 1;
	    	    	    	break;
	    	    	    }
	    	    	}
	    	    	
                        if (!user_exists) {
			    sprintf(result, "User %s tidak ditemukan", c2);
                        } 
                        else {
                            delete_row(userpath, c2);
                            reorder_ids(userpath);
		    	    sprintf(result, "%s berhasil dihapus", c2);
                    	}
	    	    }
	    	    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk menghapus user %s", c2);
	    	    }
	    	}
	    	else {
	    	    sprintf(result, "Invalid command");
	    	}
	    }
	    if(spaces == 2) {
	    	sscanf(input, "%s %s %s", c1, c2, c3);
		if ((strcmp(c1, "DEL") == 0) && (strcmp(c2, "CHANNEL") == 0)) {
		    char rankch[40];
		    char authpath[256];
		    sprintf(authpath, "%s/%s/admin/auth.csv", dcpath, c3);
		    FILE *fauth = fopen(authpath, "r+");
		    while(fgets(line, sizeof(line), fauth)) {
		    	if(strstr(line, name)) {
		    	    char *rank_channel = split_comma(line, 3);
		    	    sprintf(rankch, "%s", rank_channel);
		    	    break;
		    	}
		    }
		    fclose(fauth);
		    
		    if((strstr(rank, "ROOT")) || (strstr(rankch, "ROOT")) || (strstr(rankch, "ADMIN"))) {
		    //if(strstr(rank, "ROOT")) {
                        char chpath[256];
                        sprintf(chpath, "%s/channels.csv", dcpath);
                    
                        char temppath[256];
                        sprintf(temppath, "%s/temp.csv", dcpath);
                    
                        int channel_exists = 0;
                        FILE *fch = fopen(chpath, "a+");
                        if (!fch) {
                            perror("Error opening channels file");
                            continue;
                        }

                        char line[256];
                        while (fgets(line, sizeof(line), fch)) {
                            if (strstr(line, c3)) {
                                channel_exists = 1;
                                break;
                            }
                        }

                        if (!channel_exists) {
			    sprintf(result, "Channel %s tidak ditemukan", c3);
                        } 
                        else {
                            delete_row(chpath, c3);
                            reorder_ids(chpath);

                            char path[256];
                            snprintf(path, sizeof(path), "%s/%s", dcpath, c3);
                            if (directory_exists(path)) {
                                int ret = remove_directory(path);
                                if (ret == 0) {
		    		    sprintf(result, "%s berhasil dihapus", c3);
                                } 
                                else {
                                    sprintf(result, "Failed to delete channel %s", c3);
                                }
                            } 
                            else {
			    	sprintf(result, "Channel %s tidak ditemukan", c3);
                            }
                    	}
		    }
		    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk menghapus channel %s", c3);
		    	//sprintf(result, "%s", rankch);
		    }
                }
		else if ((strcmp(c1, "CREATE") == 0) && (strcmp(c2, "ROOM") == 0)) {
		    //sprintf(result, "create room");
		
		    if(strcmp(channel, "") != 0) {
			char rankch[40];
			char authpath[256];
			sprintf(authpath, "%s/%s/admin/auth.csv", dcpath, channel);
			FILE *fauth = fopen(authpath, "r+");
			while(fgets(line, sizeof(line), fauth)) {
			    if(strstr(line, name)) {
			        char *rank_channel = split_comma(line, 3);
			        sprintf(rankch, "%s", rank_channel);
			        break;
			    }
			}
			fclose(fauth);
			    
			if((strstr(rank, "ROOT")) || (strstr(rankch, "ROOT")) || (strstr(rankch, "ADMIN"))) {
			    char roompath[256];
			    sprintf(roompath, "%s/%s/%s", dcpath, channel, c3);
			    if(mkdir(roompath, 0777) ==  -1) {
				printf("Can't create room"); 
			    }
			    
			    char chatpath[256];
			    sprintf(chatpath, "%s/%s/%s/chat.csv", dcpath, channel, c3);
			    FILE *fchat = fopen(chatpath, "a+");
			    fclose(fchat);
			    
			    sprintf(result, "Room %s dibuat", c3);
			}
			else {
			    sprintf(result, "Anda tidak mempunyai akses untuk membuat room %s", c3);
			    //sprintf(result, "%s", rankch);
			}
		    }
		    else {
		    	sprintf(result, "Anda belum masuk channel");
		    }
		}
	    }	    
	    if(spaces == 3) {
	    	sscanf(input, "%s %s %s %s", c1, c2, c3, c4);
		//sprintf(result, "2 spaces");
	    }
	    if(spaces == 4) {
	    	sscanf(input, "%s %s %s %s %s", c1, c2, c3, c4, c5);
	    	
	    	if((strcmp(c1, "CREATE") == 0) && (strcmp(c2, "CHANNEL") == 0) && (strcmp(c4, "-k") == 0)) { //create channel
	    	    //if(strstr(rank, "ROOT")) {
		    char chpathd[256];
		    sprintf(chpathd, "%s/%s", dcpath, c3);
			    
		    char chpath[256];
		    sprintf(chpath, "%s/channels.csv", dcpath);
		    
		    FILE *fch = fopen(chpath, "a+");
		    int channel_exists = 0;
    		    int id = 1;
		    while(fgets(line, sizeof(line), fch)) {
	       	        if(strstr(line, c3)) {
	    	    	    channel_exists = 1;
	    	        }
	    	        id++;
	    	    }
			    
		    if((directory_exists(chpathd) == 0) && (channel_exists == 0)) {
			if(mkdir(chpathd, 0777) ==  -1) {
			    printf("Can't create channel"); 
			}
			
			char *bch = crypt(c5, SALT);
			
			if (bch == NULL) {
			  perror("crypt");
			  return 1;
			}
					
			fprintf(fch, "%d,%s,%s\n", id, c3, bch);
			fclose(fch);
			
			char adminpath[256];
			sprintf(adminpath, "%s/admin", chpathd);
			
			if(mkdir(adminpath, 0777) ==  -1) {
			    printf("Can't create directory for administrator"); 
			}
			
			char authpath[256];
			sprintf(authpath, "%s/auth.csv", adminpath);
			FILE *fauth = fopen(authpath, "a");
			
		        char admin_name[40];
		        sprintf(admin_name, "%s", name);
		        
			if(strstr(rank, "ROOT")) {
			    fprintf(fauth, "1,%s,ROOT\n", name);
			}
			else {
			    fprintf(fauth, "1,%s,ADMIN\n", admin_name);
			}
			fclose(fauth);
			
			char logpath[256];
			sprintf(logpath, "%s/user.log", adminpath);
			FILE *flog = fopen(logpath, "a");
			fclose(flog);
			
			sprintf(result, "Channel %s dibuat", c3);
		    }
		    else {
		        sprintf(result, "Channel %s sudah ada", c3);
		    }
		    //}
		    //else {
		    	//sprintf(result, "Anda tidak mempunyai akses untuk membuat channel %s", c3);
		    //}
	    	}
		else if((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "CHANNEL") == 0) && (strcmp(c4, "TO") == 0)) { //edit channel
		    char rankch[40];
		    char authpath[256];
		    sprintf(authpath, "%s/%s/admin/auth.csv", dcpath, c3);
		    FILE *fauth = fopen(authpath, "r+");
		    while(fgets(line, sizeof(line), fauth)) {
		    	if(strstr(line, name)) {
		    	    char *rank_channel = split_comma(line, 3);
		    	    sprintf(rankch, "%s", rank_channel);
		    	    break;
		    	}
		    }
		    fclose(fauth);
		    
		    if((strstr(rank, "ROOT")) || (strstr(rankch, "ROOT")) || (strstr(rankch, "ADMIN"))) {
                        char chpathd[256];
                        sprintf(chpathd, "%s/%s", dcpath, c3);
                    
                        char chpath[256];
                        sprintf(chpath, "%s/channels.csv", dcpath);
                    
                        FILE *fch = fopen(chpath, "a+");
                        if (fch == NULL) {
                            sprintf(result, "Channel %s tidak ditemukan", c3);
                            send(new_socket, result, sizeof(result), 0);
                            continue;
                        }
                    
                        int channel_exists = 0;
                        while(fgets(line, sizeof(line), fch)) {
                            if(strstr(line, c3)) {
                                channel_exists = 1;
                            }
                        }
                        fclose(fch);
                    
                        if((directory_exists(chpathd) == 1) && (channel_exists == 1)) {
                            edit_channel(chpath, c3, c5);  
                        
                            char newpath[256];
                            sprintf(newpath, "%s/%s", dcpath, c5);
                            rename(chpathd, newpath);
                        
                            sprintf(result, "Channel %s berhasil diubah menjadi %s", c3, c5);
                        }
                        else {
                            sprintf(result, "Channel %s tidak ditemukan", c3);
                        }
                    }
                    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk mengubah channel %s", c3);
                    }
                }
		else if((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "WHERE") == 0) && (strcmp(c4, "-u") == 0)) { //edit channel
		    if(strstr(rank, "ROOT")) {
	    	    	char userpath[256];
	    	    	sprintf(userpath, "%s/users.csv", dcpath);
	    	    	
	    	    	int user_exists = 0;
	    	    	
		        FILE *fuser = fopen(userpath, "r");
	    	    	char line[256];
	    	    	while(fgets(line, sizeof(line), fuser)) {
	    	    	    if(strstr(line, c3)) {
	    	    	    	user_exists = 1;
	    	    	    	break;
	    	    	    }
	    	    	}
	    	    	
                        if (!user_exists) {
			    sprintf(result, "User %s tidak ditemukan", c3);
                        } 
                        else {
                            edit_username(userpath, c3, c5);
		    	    sprintf(result, "%s berhasil diubah menjadi %s", c3, c5);
                    	}
                    }
                    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk mengubah user %s", c3);
                    }
                }
		else if((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "WHERE") == 0) && (strcmp(c4, "-p") == 0)) { //edit channel
		    if(strstr(rank, "ROOT")) {
	    	    	char userpath[256];
	    	    	sprintf(userpath, "%s/users.csv", dcpath);
	    	    	
	    	    	int user_exists = 0;
	    	    	
		        FILE *fuser = fopen(userpath, "r");
	    	    	char line[256];
	    	    	while(fgets(line, sizeof(line), fuser)) {
	    	    	    if(strstr(line, c3)) {
	    	    	    	user_exists = 1;
	    	    	    	break;
	    	    	    }
	    	    	}
	    	    	
                        if (!user_exists) {
			    sprintf(result, "User %s tidak ditemukan", c3);
                        } 
                        else {
			    char *buser = crypt(c5, SALT);
			
			    if (buser == NULL) {
			      perror("crypt");
			      return 1;
			    }
			    
                            edit_password(userpath, c3, buser);
		    	    sprintf(result, "password %s berhasil diubah", c3);
                    	}
                    }
                    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk mengubah user %s", c3);
                    }
                }
	    	else {
	    	    sprintf(result, "Invalid command");
	    	}
	    }
	    
            send(new_socket, result, sizeof(result), 0);
	    strcpy(result, "");
            close(new_socket);
        }
        sleep(1);
    }

    return 0;
}
