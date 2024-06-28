# Sisop-FP-2024-MH-IT03

## ***KELOMPOK IT 03***
| Nama      | NRP         |
  |-----------|-------------|
  | Haidar Rafi Aqyla | 5027231029   |
  | Hasan | 5027231073  |  
  | Muhammad kenas Galeno Putra  | 5027231069  |

# Final Praktikum Sistem Operasi

Link ketentuan final project praktikum sistem operasi bisa diakses [disini](https://its.id/m/SisopFunProject).

# Penjelasan Program

## 1. discorit.c
Program discorit.c berfungsi sebagai client dari program DiscorIT. Program ini memungkinkan user untuk menggunakan command fungsionalitas dari aplikasi DiscorIT melalui command interface yang disediakan dan mengirimkannya ke server. Program ini nanti juga akan menampilkan output dari server setelah mengirim sebuah command.

## 2. server.c
Program server.c berfungsi sebagai server dari program DiscorIT. Program ini menerima command beserta input - input lainnya (seperti nama user, rank/role user, dan sebagainya) dari client yang nantinya akan diproses sesuai dengan command. Untuk result/output dari program ini akan dikirimkan ke client.

## 3. monitor.c
Program monitor.c berfungsi untuk memantau chat dari suatu room di sebuah channel secara real time. Artinya, segala perubahan yang ada pada chat akan langsung ditampilkan pada interface program ini.

# Penjelasan Fungsionalitas DiscorIT

## 1. Register dan login
Untuk register, user dapat menjalankan program dengan command `./discorit REGISTER <nama_user> -p <pass>`. Setelah register, nama dari user akan langsung ditambahkan di file users.csv dan untuk passwordnya akan diencyrpt menggunakan algoritma hash Bcrypt.

Code:
```
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
```

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/7cfb1239-5ddb-412a-9c3c-3416d72a92d6)

Hasil (setelah register):

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/4e8a767e-d7d8-4706-8bbf-86a1259b06c3)

Untuk login, user dapat menjalankan program dengan command `./discorit LOGIN <nama_user> -p <pass>`. Password yang dimasukkan akan diencrypt dengan Bcrypt untuk dicocokkan dengan password yang ada di users.csv.

Code:
```
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
          ....
    	else {
    	    printf("%s gagal login\n", argv[2]);
    	}
```

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/bb2da71a-bba0-4688-9a3b-21341bd7d6eb)

Jika gagal login karena password salah:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/54107239-ff2c-4d5a-85be-cbebafab6aec)

## 2. List Channel

Untuk melihat daftar dari channel - channel yang ada di DiscorIT, user dapat menggunakan command `LIST CHANNEL` setelah login. Untuk implementasi dari fungsionalitas list channel menggunakan bantuan function `split_comma` yang berfungsi untuk memisahkan string berdasarkan koma. Setiap line yang diread dari file channels.csv akan dicari nama dari channel yang nantinya akan ditambahkan (concatenate) ke string `result` yang nantinya akan dikirim ke client untuk ditampilkan.

Code:
```
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
```

Utility function:
```
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
```

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/8cd8c151-2ee9-4f11-81cf-d9f8c4a74519)

## 3. Join Channel dan Room

Untuk join channel maupun room, bisa dilakukan dengan menjalankan command `JOIN <nama channel/room>` setelah login. Jika channel atau room yang ingin dimasuki tidak ada, maka akan ada output bahwa channel atau room tersebut tidak ada.

Code:
```
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
	    	        
	    	        if(channel_exists == 1) { //proses masuk channel
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
			    	    
				    char logpath[256];
				    sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
				    time_t current_time = time(NULL);
				    struct tm *local_time = localtime(&current_time);
				    char date[25];
				    strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
				    FILE *flog = fopen(logpath, "a+");
				    fprintf(flog, "[%s] %s masuk ke channel %s\n", date, name, c2);
				    fclose(flog);
			    	}
			    	else if(can_pass != 0) {
			    	    printf("Gagal masuk channel %s\n", c2);
			    	}
			    }
			    else if((user_in_channel == 1) || (strstr(rank, "ROOT"))) { //jika user sudah ada auth di channel atau merupakan root
				char rankch[40];
				char authpath[256];
				sprintf(authpath, "%s/%s/admin/auth.csv", dcpath, c2);
				FILE *fauth = fopen(authpath, "r+");
				while(fgets(line, sizeof(line), fauth)) {
			    	    if(strstr(line, name)) {
			        	char *rank_channel = split_comma(line, 3);
			        	sprintf(rankch, "%s", rank_channel);
			        	break;
			    	    }
				}
				fclose(fauth);
				
			    	if(strstr(rankch, "BANNED")) { //jika user diban di channel
			    	    printf("Anda telah diban. Silahkan menghubungi admin\n");
			    	}
			    	else {
			   	    sprintf(right, "[%s/%s]", name, c2);
			    	    strcpy(channel, c2);
			    	    is_channel = 1;
			    	    
				    char logpath[256];
				    sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
				    time_t current_time = time(NULL);
				    struct tm *local_time = localtime(&current_time);
				    char date[25];
				    strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
				    FILE *flog = fopen(logpath, "a+");
				    fprintf(flog, "[%s] %s masuk ke channel %s\n", date, name, c2);
				    fclose(flog);
			    	}
			    }
	    	        }
	    	        else {
	    	            printf("Channel %s tidak ditemukan\n", c2);
	    	        }
	    	        //comment this if this doesn't work
	    	        channel_exists = 0;
	    	    }
	    	    else if(is_channel == 1 && is_room == 0) { //room
			char chdir[256];
			sprintf(chdir, "%s/%s", dcpath, channel);
		        struct dirent *de;
		  
		        DIR *dr = opendir(chdir); 
		  
		        if (dr == NULL) { 
			    printf("Could not open current directory" ); 
		        } 
		   
		        while ((de = readdir(dr)) != NULL) {
			    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, "admin") == 0) {
			    	continue;
			    }
			    if(strcmp(de->d_name, c2) == 0) {
			    	room_exists = 1;
			    }
		  	}
		  	if(room_exists == 1) { 
	    	    	    sprintf(right, "[%s/%s/%s]", name, channel, c2);
	    	    	    strcpy(room, c2);
	    	    	    is_room = 1;
	    	    	    
			    char logpath[256];
			    sprintf(logpath, "%s/%s/admin/user.log", dcpath, channel);
				
			    time_t current_time = time(NULL);
			    struct tm *local_time = localtime(&current_time);
			    char date[25];
			    strftime(date, 25, "%d/%m/%Y %H:%M:%S", local_time);
				
			    FILE *flog = fopen(logpath, "a+");
			    fprintf(flog, "[%s] %s masuk ke room %s\n", date, name, c2);
			    fclose(flog);
		  	}
		  	else {
		  	    printf("Room tidak ditemukan\n");
		  	}
		  	//comment this if this doesn't work
		  	room_exists = 0;
	    	    }
		}
```

Jika user yang ingin masuk channel belum ada di auth.csv pada channel tersebut, maka user akan dimintai untuk memasukkan key untuk mengakses channel tersebut. Jika user sudah ada di auth.csv, maka user dapat mengakses channel tersebut secara langusng tanpa harus memasukkan key.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/e1a43521-7865-449f-9bc9-071d9dfb2548)

Jika memasukkan key yang salah:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/908c7751-900c-4e98-a8bd-31d4506f53c4)

Jika ingin memasuki channel maupun room yang tidak ada:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/0fbe6505-e1d0-40f0-8c9e-e522d1330ccf)

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/d5d5d2c0-13d0-4b1a-89be-e310850ddb11)


## 4. List User

Untuk melihat daftar dari user - user yang ada di DiscorIT maupun di channel tertentu, dapat menggunakan command `LIST USER`.

Code:
```
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
```

Untuk melihat seluruh user yang ada di DiscorIT, hanya root saja yang bisa menggunakan command `LIST USER`. Sedangkan untuk melihat seluruh user yang ada di sebuah channel, semua user dapat menggunakan command `LIST USER`.
Code ini akan mencari seluruh nama dari user yang ada di file users.csv (untuk user di DiscorIT) atau auth.csv (untuk user di channel) menggunakan function `split_comma`.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/637a287b-235b-49d2-89ee-586fa7843389)

Jika melihat seluruh user yang ada di DiscorIT (dengan user root):

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/f53faa9b-58c5-4f0f-9f36-b40761f72d3e)

## 5. List Room

Untuk list seluruh room yang ada di sebuah channel, dapat menggunakan command `LIST ROOM` dalam sebuah channel.

Code:
```
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
```

Code ini akan menampilkan seluruh directory yang merepresentasikan room yang ada di dalam directory channel. Untuk setiap room yang ada di directory channel akan ditambahkan ke output.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/8a77eade-8a78-4a70-b6ff-439655b99edd)

Jika belum masuk channel:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/aa7f39e7-7026-4824-9bc7-86ae9ebb76ae)

## 6. Chat

User dapat mengirim chat pada room dengan command `CHAT "<text>"` setelah masuk di sebuah room. Setelah chat dikirim, maka chat akan langsung ditulis di file chat.csv pada directory room beserta dengan tanggal dan waktu, id, serta username dari pengirim.

Code:

```
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
```

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/de371746-709a-4d2a-97b9-712dfb10a709)

Isi dari file chat.csv setelah chat dikirim:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/fcdc757a-d102-4087-82b2-f8b227e5b0b6)

Jika mencoba untuk chat di luar room:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/850f866f-73d9-4382-a10d-f551cd48fc32)

## 7. See Chat

User dapat menampilkan seluruh chat pada sebuah room dengan menggunakan command `SEE CHAT` setelah masuk room.

Code:

```
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
```

Utility Function:
```
void split_view(char* str, char** c1, char** c2, char** c3, char** c4) {
    *c1 = strtok(str, ",");
    *c2 = strtok(NULL, ",");
    *c3 = strtok(NULL, ",");
    *c4 = strtok(NULL, ",");
}
```

Code ini akan membaca dan menampilkan isi dari file chat.csv di dalam directory room. Sedangkan function split_view berfungsi untuk memisahkan line yang didapatkan dari file chat.csv pada directory room.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/58ae42ab-48f1-4221-91b8-5b481dbca2c4)

Jika digunakan di luar room:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/9d616f02-42aa-472c-b7a0-593332143b7f)

## 8. Edit Chat

Untuk edit chat, user dapat menggunakan command `EDIT CHAT <id> "<chat_baru>"`.

Code:
```
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
```

Utility function:
```
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
```

Untuk code edit chat ini menggunakan bantuan dari function `edit_chat`. Function `edit_chat` berfungsi untuk mencari line dari chat berdasarkan id, kemudian mengganti isi dari chat itu dengan menggunakan string token.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/9f25a1c5-9731-4042-ba91-479c456d841f)

Hasil dari edit pada file chat.csv:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/0cdecf19-a745-424f-87f1-b451b0dff5c6)

Jika digunakan di luar room:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/bcc798f7-526a-4133-b0d8-c9a067ce6bc5)

## 9. Delete Chat

Untuk delete chat pada room, user dapat menggunakan command `DEL CHAT <id>`. Untuk command ini, hanya root atau admin yang dapat menggunakan.

Code:
```
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
```

Pada code ini, rank/role channel dari user akan dicek terlebih dahulu dengan bantuan function `split_comma`. Jika user adalah admin pada channel atau root, maka nanti akan bisa menghapus chat. Namun, jika user adalah user biasa, maka user tidak bisa delete chat. Code ini menggunakan bantuan function `delete_chat` dan `reorder_dhat`.

Utility function:
```
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
```

Function `delete_chat` berfungsi untuk menghapus baris chat berdasarkan ID dari chat. Sedangkan untuk function `reorder_chat` berfungsi untuk mengurutkan kembali ID chat pada file chat.csv pada directory room.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/f8095462-3862-4f65-b94b-54f148d8492e)

Hasil pada file csv setelah delete:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/c48e6d32-1731-47a0-8deb-d6311b8c2333)

Jika digunakan oleh user biasa:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/20e156c4-008d-4de8-b4fd-964a96ce596b)

Jika digunakan di luar room:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/1c4ee957-7e15-43f9-aaf1-75c4a7732c05)

## 10. Edit User

Untuk edit username maupun password yang ada di DiscorIT, dapat menggunakan command `EDIT WHERE <username> -u <new_username>` untuk mengganti username dari sebuah user atau `EDIT WHERE <username> -p <new_password>` untuk mengganti password dari sebuah user.
Untuk command ini, hanya user root saja yang bisa menggunakan.

Code:
```
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
```

Code ini akan mengubah nama dari user (untuk command `EDIT WHERE <username> -u <new_username>' tidak hanya di file users.csv saja, tetapi juga username yang ada di file auth.csv di dalam directory channel. Untuk code ini menggunakan bantuan function edit_username dan edit_password.

Utility function:
```
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
```

Kedua function ini menggunakan string token untuk mengubah username dan password.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/b5668b16-5274-4046-aa55-98a94b20cc7d)

Hasil setelah edit (username dan password):

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/dcec1121-6b8e-4d46-869d-be83f061e345)

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/a3a6729b-bf4c-4e61-b9a2-5a7748c819d5)

Isi dari file users.csv:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/00a17086-10cf-41ed-842a-850d1ab7ffe4)

Isi dari file auth.csv pada directory channel sisop:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/29f20149-6cc7-4c5a-923e-dcf6b23438db)

Jika digunakan oleh user biasa:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/9c7e5113-190d-477c-9b36-751da256ffde)

