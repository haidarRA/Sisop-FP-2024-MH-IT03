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


