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
Code ini akan mencari seluruh nama dari user yang ada di file users.csv (untuk user di DiscorIT) atau auth.csv (untuk user di channel) menggunakan function split_comma.

Demonstrasi:

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/637a287b-235b-49d2-89ee-586fa7843389)

Jika melihat seluruh user yang ada di DiscorIT (dengan user root):

![image](https://github.com/haidarRA/Sisop-FP-2024-MH-IT03/assets/149871906/f53faa9b-58c5-4f0f-9f36-b40761f72d3e)

## 5. List Room
