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

void split_view(char* str, char** c1, char** c2, char** c3, char** c4) {
    *c1 = strtok(str, ",");
    *c2 = strtok(NULL, ",");
    *c3 = strtok(NULL, ",");
    *c4 = strtok(NULL, ",");
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

void delete_chat(const char *csvpath, const char *id) {
    FILE *file = fopen(csvpath, "r");
    
    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.temp", csvpath);
    FILE *temp = fopen(temp_filepath, "w");

    if (file == NULL || temp == NULL) {
        perror("Error opening file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char check_id[30];
        sprintf(check_id, ",%s,", id);
        if (!strstr(line, check_id)) {
            fprintf(temp, "%s", line);
        }
    }

    fclose(file);
    fclose(temp);

    remove(csvpath);
    rename(temp_filepath, csvpath);
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
                token_index += strlen(new_substring) + 1;
            } else {
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", token);
                token_index += strlen(token) + 1; 
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
        char modified_line[512] = {0};
        char current_username[256] = {0};

        while ((token = strtok_r(rest, ",", &rest))) {
            count++;
            if (count == 2) {
                strncpy(current_username, token, sizeof(current_username) - 1);
            }
            if (count == 3 && strcmp(current_username, username) == 0) {
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", new_pass);
                token_index += strlen(new_pass) + 1;
            } 
            else {
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", token);
                token_index += strlen(token) + 1;
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

void edit_chat(const char *csvpath, int id, const char *new_chat) {
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
        char modified_line[512] = {0};
        int current_id = -1;

        while ((token = strtok_r(rest, ",", &rest))) {
            count++;
            if (count == 2) {
                current_id = atoi(token);
            }
            if (count == 4 && current_id == id) {
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", new_chat);
                token_index += strlen(new_chat) + 1;
            } else {
                // Copy other fields as is
                snprintf(modified_line + token_index, sizeof(modified_line) - token_index, "%s,", token);
                token_index += strlen(token) + 1;
            }
        }

        if (modified_line[token_index - 1] == ',') {
            modified_line[token_index - 1] = '\0';
        }
        
        if(current_id == id) {
            strcat(modified_line, "\n");
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

void reorder_chat(const char *filepath) {
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
        char *first_comma = strchr(line, ',');
        if (first_comma) {
            char *second_comma = strchr(first_comma + 1, ',');
            if (second_comma) {
                fprintf(temp_file, "%.*s,%d%s", (int)(first_comma - line), line, id, second_comma);
                id++;
            }
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

void split_string(const char* input, char* parts[], const char* delimiter) {
    char* temp = strdup(input);
    char* token;
    int i = 0;

    token = strtok(temp, delimiter);
    while (token != NULL && i < 7) {
        parts[i] = strdup(token);
        token = strtok(NULL, delimiter);
        i++;
    }

    free(temp);
}

void split_chat(const char *input, char **c1, char **c2) {
    const char *spacePos = strchr(input, ' ');

    if (!spacePos) {
        *c1 = strdup(input);
        *c2 = strdup("");
        return;
    }

    int c1Length = spacePos - input;
    *c1 = (char *)malloc(c1Length + 1);
    strncpy(*c1, input, c1Length);
    (*c1)[c1Length] = '\0';

    *c2 = strdup(spacePos + 1);
}

void split_1_space(char *input, char *c1, char *c2) {
    char *space_pos = strchr(input, ' ');
    if (space_pos != NULL) {
        strncpy(c1, input, space_pos - input);
        c1[space_pos - input] = '\0';

        strcpy(c2, space_pos + 1);

        size_t len = strlen(c2);
        if (len > 0 && c2[len - 1] == '\n') {
            c2[len - 1] = '\0';
        }
    }
}

void split_3_spaces(char *input, char *c1, char *c2, char *c3, char *c4) {
    char *token;
    token = strtok(input, " ");
    if (token != NULL) {
        strcpy(c1, token);
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strcpy(c2, token);
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strcpy(c3, token);
    }

    token = strtok(NULL, "\n"); 
    if (token != NULL) {
        strcpy(c4, token);
    }
}

void change_rank(const char *csvpath, const char *username, const char *new_role) {
    FILE *file = fopen(csvpath, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    char temp_filepath[256];
    snprintf(temp_filepath, sizeof(temp_filepath), "%s.temp", csvpath);
    FILE *temp = fopen(temp_filepath, "w");
    if (!temp) {
        fclose(file);
        perror("Failed to create temp file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *token;
        char *line_copy = strdup(line);

        token = strtok(line_copy, ",");
        fprintf(temp, "%s,", token);

        token = strtok(NULL, ",");
        fprintf(temp, "%s,", token);

        if (strcmp(token, username) == 0) {
            fprintf(temp, "%s\n", new_role);
        } else {
            token = strtok(NULL, "\n");
            fprintf(temp, "%s\n", token);
        }

        free(line_copy);
    }

    fclose(file);
    fclose(temp);

    remove(csvpath);
    rename(temp_filepath, csvpath);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char input[125], result[2048], rank[40], name[40], channel[40], room[40], agg[260], is_channel[10], is_room[10];
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
	    
            int bytes_read = read(new_socket, agg, sizeof(agg));
            if (bytes_read <= 0) {
                printf("Invalid data received from client\n");
                close(new_socket);
                continue;
            }
            
    	    char *splitted[7];

    	    split_string(agg, splitted, "|");
    	    sprintf(input, "%s", splitted[0]);
    	    sprintf(rank, "%s", splitted[1]);
    	    sprintf(name, "%s", splitted[2]);
    	    sprintf(is_channel, "%s", splitted[3]);
    	    sprintf(is_room, "%s", splitted[4]);
    	    sprintf(channel, "%s", splitted[5]);
    	    sprintf(room, "%s", splitted[6]);
		    
	    int spaces;
	    
	    if((strstr(input, "CHAT")) && !(strstr(input, "EDIT")) && !(strstr(input, "DEL"))) {
		spaces = 1;
	    }
	    else if((strstr(input, "CHAT")) && (strstr(input, "EDIT"))) {
		spaces = 3;
	    }
	    else {
	    	spaces = count_spaces(input);
	    }
    
	    char* u_path = get_path();

            char dcpath[256];
	    sprintf(dcpath, "%s/DiscorIT", u_path);

	    char c1[50], c2[50], c3[50], c4[50], c5[50], line[256];
	    if(spaces == 1) {
    		split_1_space(input, c1, c2);
	    	if((strcmp(c1, "LIST") == 0) && (strcmp(c2, "CHANNEL") == 0)) { //list channel
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
		    strcat(result, "\n");
		    
		    if(strstr(result, "$y")) {
			char *removed = "$y";
			char *start = strstr(result, removed);
			char *end = start + strlen(removed);
			strcpy(start, end);
    		    }
    		    
		    if(strstr(result, "����")) {
			char *removed = "����";
			char *start = strstr(result, removed);
			char *end = start + strlen(removed);
			strcpy(start, end);
    		    }

		    fclose(fch);
	    	}
		else if ((strcmp(c1, "CHAT") == 0)) { //chat
		    if (strcmp(is_room, "0") != 0) {
		    //if (room != NULL) {
			char chatpath[256];
			sprintf(chatpath, "%s/%s/%s/chat.csv", dcpath, channel, room);
		        
			int id = 1;
			FILE *fchat = fopen(chatpath, "a+");
			while (fgets(line, sizeof(line), fchat)) {
			    id++;
			}

			time_t current_time = time(NULL);
			struct tm *local_time = localtime(&current_time);
			char date[25];
			strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);

			fprintf(fchat, "%s,%d,%s,%s\n", date, id, name, c2);
			fclose(fchat);
			
			char logpath[256];
			sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
			
			FILE *flog = fopen(logpath, "a+");
			fprintf(flog, "[%s] %s chat %s di room %s\n", date, name, c2, room);
			fclose(flog);
		    } 
		    else {
			sprintf(result, "Anda masih belum masuk room\n");
		    }
		}
		else if ((strcmp(c1, "SEE") == 0) && (strcmp(c2, "CHAT") == 0)) { //see chat
		    if (strcmp(is_room, "0") != 0) {
		    //if (room != NULL) {
			char chatpath[256];
			sprintf(chatpath, "%s/%s/%s/chat.csv", dcpath, channel, room);
		        
		        char *chat_date, *chat_id, *chat_username, *chat_content, mod_line[256];
			FILE *fchat = fopen(chatpath, "r");
			while (fgets(line, sizeof(line), fchat)) {
    			    split_view(line, &chat_date, &chat_id, &chat_username, &chat_content);
    			    chat_content[strcspn(chat_content, "\n")] = '\0';
    			    
			    sprintf(mod_line, "[%s][%s][%s] %s", chat_date, chat_id, chat_username, chat_content);
			    strcat(result, mod_line);
			    strcat(result, "\n");
			    strcpy(mod_line, "");
			}
			fclose(fchat);
		    } 
		    else {
			sprintf(result, "Anda masih belum masuk room\n");
		    }
		}
	    	else if((strcmp(c1, "LIST") == 0) && (strcmp(c2, "USER") == 0)) { //list user
		    if (strcmp(is_channel, "0") == 0) { //list user di discorit 
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
		            strcat(result, "\n");

		            if(strstr(result, "$y")) {
			        char *removed = "$y";
			        char *start = strstr(result, removed);
			        char *end = start + strlen(removed);
			        strcpy(start, end);
    		            }
    		        
		            if(strstr(result, "����")) {
			        char *removed = "����";
			        char *start = strstr(result, removed);
			        char *end = start + strlen(removed);
			        strcpy(start, end);
    		            }
    		        
		    	    fclose(fuser);
    		        }
    		        else {
		        	sprintf(result, "Anda tidak mempunyai akses untuk melihat seluruh user\n");
    		        }
    		    }
		    else if (strcmp(is_channel, "0") != 0) { //list user di channel
		    	char authpath[256];
			sprintf(authpath, "%s/%s/admin/auth.csv", dcpath, channel);
			FILE *fauth = fopen(authpath, "r+");
			
		        char line[256];
			while(fgets(line, sizeof(line), fauth)) {
			    char *user_name = split_comma(line, 2);
			    strcat(result, user_name);
			    strcat(result, " ");
			}
		        strcat(result, "\n");
		        
		        if(strstr(result, "$y")) {
			    char *removed = "$y";
			    char *start = strstr(result, removed);
			    char *end = start + strlen(removed);
			    strcpy(start, end);
    		        }
    		        
		        if(strstr(result, "����")) {
			    char *removed = "����";
			    char *start = strstr(result, removed);
			    char *end = start + strlen(removed);
			    strcpy(start, end);
    		        }
		        
			fclose(fauth);
		    } 
	    	}
	    	else if(strcmp(c1, "REMOVE") == 0) { //remove user (bukan di channel, di satu discorit)
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
			    sprintf(result, "User %s tidak ditemukan\n", c2);
                        } 
                        else {
                            delete_row(userpath, c2);
                            reorder_ids(userpath);
		    	    sprintf(result, "%s berhasil dihapus\n", c2);
                    	}
	    	    }
	    	    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk menghapus user %s\n", c2);
	    	    }
	    	}
		else if ((strcmp(c1, "LIST") == 0) && (strcmp(c2, "ROOM") == 0)) { //list room
		    if (strcmp(is_channel, "0") != 0) {
		    //if (channel != NULL) {
			char chpath[256];
			sprintf(chpath, "%s/%s", dcpath, channel);
		        struct dirent *de;
		  
		        DIR *dr = opendir(chpath); 
		  
		        if (dr == NULL) { 
			    printf("Could not open current directory" ); 
		        } 
		   
		        while ((de = readdir(dr)) != NULL) {
			    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
			      continue;
			    }
			    strcat(result, de->d_name);
			    strcat(result, " ");
		  	}
		  	strcat(result, "\n");
		        closedir(dr);     
		    } 
		    else {
			sprintf(result, "Anda masih belum masuk channel\n");
		    }
		}
		else if (strcmp(c1, "BAN") == 0) { //ban user dari channel
		    if(strcmp(is_channel, "0") != 0) {
		    //if (channel != NULL) {
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
			    fauth = fopen(authpath, "r+");
			    int user_exists = 0;
			    while(fgets(line, sizeof(line), fauth)) {
			        if(strstr(line, c2)) {
				    user_exists = 1;
			            break;
			        }
			    }
			    fclose(fauth);
			    
			    if(user_exists == 1) {
    			        change_rank(authpath, c2, "BANNED");
			        sprintf(result, "%s diban\n", c2);
			        
			        char logpath[256];
			        sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
			        time_t current_time = time(NULL);
			        struct tm *local_time = localtime(&current_time);
			        char date[25];
			        strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			        FILE *flog = fopen(logpath, "a+");
			        fprintf(flog, "[%s] admin ban %s\n", date, c2);
			        fclose(flog);
			    }
			    else {
			        sprintf(result, "User %s tidak ditemukan\n", c2);
			    }
			}
			else {
			    sprintf(result, "Anda tidak mempunyai akses untuk ban user %s\n", c2);
			}
		    }
		    else {
		    	sprintf(result, "Anda belum masuk channel\n");
		    }
		}
		else if (strcmp(c1, "UNBAN") == 0) { //unban user dari channel
		    if(strcmp(is_channel, "0") != 0) {
		    //if (channel != NULL) {
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
			    fauth = fopen(authpath, "r+");
			    int user_exists = 0;
			    while(fgets(line, sizeof(line), fauth)) {
			        if(strstr(line, c2)) {
				    user_exists = 1;
			            break;
			        }
			    }
			    fclose(fauth);
			    
			    if(user_exists == 1) {
    			        change_rank(authpath, c2, "USER");
			        sprintf(result, "%s kembali\n", c2);
			        
			        char logpath[256];
			        sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
			        time_t current_time = time(NULL);
			        struct tm *local_time = localtime(&current_time);
			        char date[25];
			        strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			        FILE *flog = fopen(logpath, "a+");
			        fprintf(flog, "[%s] admin unban %s\n", date, c2);
			        fclose(flog);
			    }
			    else {
			        sprintf(result, "User %s tidak ditemukan\n", c2);
			    }
			}
			else {
			    sprintf(result, "Anda tidak mempunyai akses untuk ban user %s\n", c2);
			}
		    }
		    else {
		    	sprintf(result, "Anda belum masuk channel\n");
		    }
		}
	    	else {
	    	    sprintf(result, "Invalid command\n");
	    	}
	    }
	    if(spaces == 2) {
	    	sscanf(input, "%s %s %s", c1, c2, c3);
		if ((strcmp(c1, "DEL") == 0) && (strcmp(c2, "CHANNEL") == 0)) { //delete channel
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
			    sprintf(result, "Channel %s tidak ditemukan\n", c3);
                        } 
                        else {
                            delete_row(chpath, c3);
                            reorder_ids(chpath);

                            char path[256];
                            snprintf(path, sizeof(path), "%s/%s", dcpath, c3);
                            if (directory_exists(path)) {
                                int ret = remove_directory(path);
                                if (ret == 0) {
		    		    sprintf(result, "%s berhasil dihapus\n", c3);
                                } 
                                else {
                                    sprintf(result, "Failed to delete channel %s\n", c3);
                                }
                            } 
                            else {
			    	sprintf(result, "Channel %s tidak ditemukan\n", c3);
                            }
                    	}
		    }
		    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk menghapus channel %s\n", c3);
		    }
                }
		else if ((strcmp(c1, "CREATE") == 0) && (strcmp(c2, "ROOM") == 0)) { //create room
		    if(strcmp(is_channel, "0") != 0) {
		    //if (channel != NULL) {
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
			    
			    sprintf(result, "Room %s dibuat\n", c3);
			    
			    char logpath[256];
			    sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
			    time_t current_time = time(NULL);
			    struct tm *local_time = localtime(&current_time);
			    char date[25];
			    strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			    FILE *flog = fopen(logpath, "a+");
			    fprintf(flog, "[%s] admin buat room %s\n", date, c3);
			    fclose(flog);
			}
			else {
			    sprintf(result, "Anda tidak mempunyai akses untuk membuat room %s\n", c3);
			}
		    }
		    else {
		    	sprintf(result, "Anda belum masuk channel\n");
		    }
		}
		else if ((strcmp(c1, "DEL") == 0) && (strcmp(c2, "CHAT") == 0)) { //delete chat
		    if(strcmp(is_room, "0") != 0) {
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
		    	    char chatpath[256];
		    	    sprintf(chatpath, "%s/%s/%s/chat.csv", dcpath, channel, room);
                    
                            int line_count = 0;

                            char line[256];
                            FILE *fchat = fopen(chatpath, "a+");
                            while (fgets(line, sizeof(line), fchat)) {
			        line_count++;
                            }
                            fclose(fchat);

			    int target_id = atoi(c3);

                            if (target_id > line_count) {
			        sprintf(result, "Chat ID %s tidak ditemukan\n", c3);
                            } 
                            else {
                                delete_chat(chatpath, c3);
                                reorder_chat(chatpath);
                    	    }
		        }
		        else {
		    	    sprintf(result, "Anda tidak mempunyai akses untuk menghapus chat ID %s\n", c3);
		        }
		    }
		    else {
			sprintf(result, "Anda masih belum masuk room\n");
		    }
                }
		else if ((strcmp(c1, "DEL") == 0) && (strcmp(c2, "ROOM") == 0) && (strcmp(c3, "ALL") != 0)) { //delete room (id)
		    if(strcmp(is_channel, "0") != 0) {
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
                    
                    	    char chpath[256];
                    	    sprintf(chpath, "%s/%s", dcpath, channel);
                    	
		            struct dirent *de;
		  
		            DIR *dr = opendir(chpath); 
		  
		            if (dr == NULL) { 
			        printf("Could not open current directory" ); 
		            } 
		   	
		   	    int room_found = 0;
		            while ((de = readdir(dr)) != NULL) {
			        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
			          continue;
			        }
			        if (strcmp(de->d_name, c3) == 0) {
			        	room_found = 1;
			        }
		  	    }
		            closedir(dr);   

                            if (room_found == 1) {
                                int ret = remove_directory(roompath);
                                if (ret == 0) {
		    		    sprintf(result, "%s berhasil dihapus\n", c3);
		    		    
			            char logpath[256];
			            sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
			            time_t current_time = time(NULL);
			            struct tm *local_time = localtime(&current_time);
			            char date[25];
			            strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			            FILE *flog = fopen(logpath, "a+");
			            fprintf(flog, "[%s] admin hapus room %s\n", date, c3);
			            fclose(flog);
                                } 
                                else {
                                    sprintf(result, "Failed to delete room %s\n", c3);
                                }
                            } 
                            else {
			        sprintf(result, "Room %s tidak ditemukan\n", c3);
                    	    }
		        }
		        else {
		    	    sprintf(result, "Anda tidak mempunyai akses untuk menghapus room %s\n", c3);
		        }
		    }
		    else {
		    	sprintf(result, "Anda belum masuk channel\n");
		    }
                }
		else if ((strcmp(c1, "DEL") == 0) && (strcmp(c2, "ROOM") == 0) && (strcmp(c3, "ALL") == 0)) { //delete room (all)
		    if(strcmp(is_channel, "0") != 0) {
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
                    
                    	    char chpath[256];
                    	    sprintf(chpath, "%s/%s", dcpath, channel);
                    	
		            struct dirent *de;
		  
		            DIR *dr = opendir(chpath); 
		  
		            if (dr == NULL) { 
			        printf("Could not open current directory" ); 
		            } 
		   	
		            while ((de = readdir(dr)) != NULL) {
			        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
			          continue;
			        }
			        else {
				    sprintf(roompath, "%s/%s/%s", dcpath, channel, de->d_name);
                                    int ret = remove_directory(roompath);
                                    if (ret != 0) {
                                        sprintf(result, "Failed to delete channel %s\n", de->d_name);
                                    }
			        }
		  	    }
		            closedir(dr);   

		    	    sprintf(result, "Semua room dihapus\n");

		            char logpath[256];
		            sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
			
		            time_t current_time = time(NULL);
		            struct tm *local_time = localtime(&current_time);
		            char date[25];
		            strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
			
		            FILE *flog = fopen(logpath, "a+");
		            fprintf(flog, "[%s] admin hapus semua room\n", date);
		            fclose(flog);
		        }
		        else {
		    	    sprintf(result, "Anda tidak mempunyai akses untuk menghapus semua room\n");
		        }
		    }
		    else {
		    	sprintf(result, "Anda belum masuk channel\n");
		    }
                }
		else if ((strcmp(c1, "REMOVE") == 0) && (strcmp(c2, "USER") == 0)) { //kick user dari channel
		    if(strcmp(is_channel, "0") != 0) {
		    //if (channel != NULL) {
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
                            delete_row(authpath, c3);
                            reorder_ids(authpath);
			    sprintf(result, "%s dikick\n", c3);
			    
			    char logpath[256];
			    sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
			    time_t current_time = time(NULL);
			    struct tm *local_time = localtime(&current_time);
			    char date[25];
			    strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			    FILE *flog = fopen(logpath, "a+");
			    fprintf(flog, "[%s] admin kick %s\n", date, c3);
			    fclose(flog);
			}
			else {
			    sprintf(result, "Anda tidak mempunyai akses untuk kick user %s\n", c3);
			}
		    }
		    else {
		    	sprintf(result, "Anda belum masuk channel\n");
		    }
		}
	    	else {
	    	    sprintf(result, "Invalid command\n");
	    	}
	    }	    
	    if(spaces == 3) {
    		split_3_spaces(input, c1, c2, c3, c4);
    		
		if ((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "CHAT") == 0)) { //edit chat
		    if (strcmp(is_room, "0") != 0) {
		    //if (room != NULL) {
			char chatpath[256];
			sprintf(chatpath, "%s/%s/%s/chat.csv", dcpath, channel, room);

			int id_chat = atoi(c3);

			edit_chat(chatpath, id_chat, c4);
		    } 
		    else {
			sprintf(result, "Anda masih belum masuk room\n");
		    }
		}
	    	else {
	    	    sprintf(result, "Invalid command\n");
	    	}
	    }
	    if(spaces == 4) {
	    	sscanf(input, "%s %s %s %s %s", c1, c2, c3, c4, c5);
	    	
	    	if((strcmp(c1, "CREATE") == 0) && (strcmp(c2, "CHANNEL") == 0) && (strcmp(c4, "-k") == 0)) { //create channel
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
			
			sprintf(result, "Channel %s dibuat\n", c3);
		    }
		    else {
		        sprintf(result, "Channel %s sudah ada\n", c3);
		    }
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
                            sprintf(result, "Channel %s tidak ditemukan\n", c3);
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
                        
                            sprintf(result, "Channel %s berhasil diubah menjadi %s\n", c3, c5);
                            
			    char logpath[256];
			    sprintf(logpath, "%s/%s/admin/user.log", dcpath, c5);
				
			    time_t current_time = time(NULL);
			    struct tm *local_time = localtime(&current_time);
			    char date[25];
			    strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			    FILE *flog = fopen(logpath, "a+");
			    fprintf(flog, "[%s] admin edit channel %s jadi %s\n", date, c3, c5);
			    fclose(flog);
                        }
                        else {
                            sprintf(result, "Channel %s tidak ditemukan\n", c3);
                        }
                    }
                    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk mengubah channel %s\n", c3);
                    }
                }
		else if((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "WHERE") == 0) && (strcmp(c4, "-u") == 0)) { //edit nama user
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
			    sprintf(result, "User %s tidak ditemukan\n", c3);
                        } 
                        else {
                            edit_username(userpath, c3, c5);
			    struct dirent *de;
			  
			    DIR *dr = opendir(dcpath); 
			  
			    if (dr == NULL) { 
				printf("Could not open current directory" ); 
			    } 
			   	
			    while ((de = readdir(dr)) != NULL) {
				if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
				    continue;
				}
				else {
				    char user_channel[256];
				    sprintf(user_channel, "%s/%s/admin/auth.csv", dcpath, de->d_name);
				    edit_username(user_channel, c3, c5);
				}
			    }
			    closedir(dr);   
		    	    sprintf(result, "%s berhasil diubah menjadi %s\n", c3, c5);
                    	}
                    }
                    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk mengubah user %s\n", c3);
                    }
                }
		else if((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "WHERE") == 0) && (strcmp(c4, "-p") == 0)) { //edit pass user
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
			    sprintf(result, "User %s tidak ditemukan\n", c3);
                        } 
                        else {
			    char *buser = crypt(c5, SALT);
			
			    if (buser == NULL) {
			      perror("crypt");
			      return 1;
			    }
			    
                            edit_password(userpath, c3, buser);
		    	    sprintf(result, "password %s berhasil diubah\n", c3);
                    	}
                    }
                    else {
		    	sprintf(result, "Anda tidak mempunyai akses untuk mengubah user %s\n", c3);
                    }
                }
		else if((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "PROFILE") == 0) && (strcmp(c3, "SELF") == 0) && (strcmp(c4, "-u") == 0)) { //edit nama sendiri
		    char userpath[256];
		    sprintf(userpath, "%s/users.csv", dcpath);
		    
		    edit_username(userpath, name, c5);
		    
		    struct dirent *de;
			  
		    DIR *dr = opendir(dcpath); 
			  
		    if (dr == NULL) { 
			printf("Could not open current directory" ); 
		    } 
		   	
		    while ((de = readdir(dr)) != NULL) {
			if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
			    continue;
			}
			else {
			    char user_channel[256];
			    sprintf(user_channel, "%s/%s/admin/auth.csv", dcpath, de->d_name);
			    edit_username(user_channel, name, c5);
			}
		    }
		    closedir(dr);   
		    
		    sprintf(result, "%s|Profil diupdate\n", c5);
                }
		else if((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "PROFILE") == 0) && (strcmp(c3, "SELF") == 0) && (strcmp(c4, "-p") == 0)) { //edit pass sendiri
		    char userpath[256];
		    sprintf(userpath, "%s/users.csv", dcpath);
		    
		    char *buser = crypt(c5, SALT);
			
		    if (buser == NULL) {
		      perror("crypt");
		      return 1;
		    }
			    
                    edit_password(userpath, name, buser);
		    sprintf(result, "Profil diupdate\n");
                }
		else if ((strcmp(c1, "EDIT") == 0) && (strcmp(c2, "ROOM") == 0) && (strcmp(c4, "TO") == 0)) { //edit room
		    if (strcmp(is_channel, "0") != 0) {
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
			    char old_path[256];
			    sprintf(old_path, "%s/%s/%s", dcpath, channel, c3);
			
			    char new_path[256];
			    sprintf(new_path, "%s/%s/%s", dcpath, channel, c5);
                    
                        	char chpath[256];
                    	    sprintf(chpath, "%s/%s", dcpath, channel);
                    	
		            struct dirent *de;
		  
		            DIR *dr = opendir(chpath); 
		  
		            if (dr == NULL) { 
			        printf("Could not open current directory" ); 
		            } 
		   	
		   	    int room_found = 0;
		            while ((de = readdir(dr)) != NULL) {
			        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
			          continue;
			        }
			        if (strcmp(de->d_name, c3) == 0) {
			    	    room_found = 1;
			        }
		  	    }
		            closedir(dr);   

                            if (room_found == 1) {
                                rename(old_path, new_path);
			        sprintf(result, "%s berhasil diubah menjadi %s\n", c3, c5);
			        
			        char logpath[256];
			        sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
			        time_t current_time = time(NULL);
			        struct tm *local_time = localtime(&current_time);
			        char date[25];
			        strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			        FILE *flog = fopen(logpath, "a+");
			        fprintf(flog, "[%s] admin edit room %s jadi %s\n", date, c3, c5);
			        fclose(flog);
                            } 
                            else {
			        sprintf(result, "Room %s tidak ditemukan\n", c3);
                    	    }
		        }
		        else {
		    	    sprintf(result, "Anda tidak mempunyai akses untuk edit room %s\n", c3);
		        }
		    }
		    else {
			sprintf(result, "Anda masih belum masuk channel\n");
		    }
                }
	    	else {
	    	    sprintf(result, "Invalid command\n");
	    	}
	    }
	    
	    if(strstr(result, "����")) {
		char *removed = "����";
		char *start = strstr(result, removed);
		char *end = start + strlen(removed);
		strcpy(start, end);
    	    }
	    
            send(new_socket, result, sizeof(result), 0);
	    strcpy(result, "");
            close(new_socket);
        }
        sleep(1);
    }

    return 0;
}
