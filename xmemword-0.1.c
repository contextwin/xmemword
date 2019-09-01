#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/Xlocale.h>

#define WIDTH 520
#define HEIGHT 350
#define COL 30
#define ROW 20
#define STRINGS_MAX 1024
#define Q_SCREEN_1 0
#define Q_SCREEN_2 1

#define PATH_MAX 4096
#define NAME_MAX 255
#define FILES_DIR_NAME "/Files/" // 出題ファイル格納ディレクトリ
#define FILES_MAX 256 // 取り扱う出題ファイル数の最大
#define STRINGS_MAX 1024 // 一つの文字列の最大
#define QUESTION_MAX 2000 // 一つのファイルの最大出題数
#define SUCCESS 0
#define ERROR 1
#define EDITOR "vi"

Display *disp;
Window root, lab, moniter_frame, question_moniter, user_input_moniter;

GC gc1, gc2, gc3;    // グラフィックコンテキストの略、色や線の太さなどを指定する
XFontSet ja_fs;            // 日本語フォントセット 

XIM im;

char *start_menu_str2 = "数値を入力して下さい。";
char *start_menu_select1 = "[  0] プログラム終了。";
char *start_menu_select2 = "[  1] 暗記を始める。";
char *start_menu_select3 = "[  2] 出題ファイルを編集する。";

unsigned int char_cnt = 0;
static int input_position = 0;
char user_input_strings[STRINGS_MAX];

/* struct */
struct FilelistStruct {
 unsigned char file_number;
 char file_name[NAME_MAX];
};

struct AnswerAndQuestion {
 unsigned long number;
 unsigned long rand_key;
 char answer[STRINGS_MAX];
 char question[STRINGS_MAX];
};

/* function */
unsigned int fp_read_and_split(FILE *fp,
 struct AnswerAndQuestion *answer_and_question_s) {

 unsigned char cnt = 0, cnt1 = 0;
 unsigned short question_max = 0;

 for (cnt = 0; !feof(fp); cnt++) {
  answer_and_question_s[cnt].number = cnt + 1;

  for(cnt1 = 0 ;; cnt1++){
   answer_and_question_s[cnt].answer[cnt1] = getc(fp);

   if (answer_and_question_s[cnt].answer[cnt1] == '\t') {
    answer_and_question_s[cnt].answer[cnt1] = '\0';
    break;
   }

   if (feof(fp)) break;

  }

  for (cnt1 = 0; answer_and_question_s[cnt]. question[cnt1] != '\n'; cnt1++) {
   answer_and_question_s[cnt].question[cnt1] = getc(fp);

   if (answer_and_question_s[cnt].question[cnt1] == '\n') {
    answer_and_question_s[cnt].question[cnt1] = '\0';
    question_max++;
    break;
   }

   if (feof(fp)) break;

  }
  
 }

 fclose(fp);
 return question_max;
}

/* view function */
void ClearQuestionMoniter() {
 XClearArea(disp, question_moniter, 3,  2, 440, 16, False);
 XClearArea(disp, question_moniter, 3, 17, 440, 16, False);
 XClearArea(disp, question_moniter, 3, 37, 440, 16, False);
 XClearArea(disp, question_moniter, 3, 57, 440, 16, False);
 XClearArea(disp, question_moniter, 3, 77, 440, 16, False);
 XClearArea(disp, question_moniter, 3, 97, 440, 16, False);
 XClearArea(disp, question_moniter, 3, 117, 440, 16, False);
 XClearArea(disp, question_moniter, 3, 137, 440, 16, False);
};

void DrawQuestionMoniterJp(unsigned char q_screen) {

 if (q_screen == Q_SCREEN_1) {
  XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 30,
                start_menu_str2, strlen(start_menu_str2));
  XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 50,
                start_menu_select1, strlen(start_menu_select1));
  XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 70,
                start_menu_select2, strlen(start_menu_select2));
  XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 90,
                start_menu_select3, strlen(start_menu_select3));
 }

};

void UserInputMoniterClear() {
 char_cnt = 0;
 input_position = 0;
 XClearArea(disp, user_input_moniter, 16, 15, 400, 16, False);
};

void DeleteCharacter() {
 printf("%d\n", char_cnt);
 printf("%d\n", input_position);
 printf("%s\n", user_input_strings);

 if (input_position >= 7) {
  XClearArea(disp, user_input_moniter, (input_position + 8), 15, 10, 16, False);
  input_position -= 7;
  char_cnt = (char_cnt - 1);
  user_input_strings[char_cnt] = '\0';
 }

};

void ExitProgram() {
 printf("ProgramExit\n");
 XCloseIM(im);
 XDestroyWindow(disp, root);
 XCloseDisplay(disp);
 exit(EXIT_SUCCESS);
};

unsigned int FpReadAndSplit(FILE *fp,
 struct AnswerAndQuestion *answer_and_question_s) {

 unsigned char cnt1 = 0, cnt2 = 0;
 unsigned int question_max = 0;

 for (cnt1 = 0; !feof(fp); cnt1++) {
  answer_and_question_s[cnt1].number = cnt1 + 1;

  for (cnt2 = 0 ;; cnt2++) {
   answer_and_question_s[cnt1].answer[cnt2] = getc(fp);

   if (answer_and_question_s[cnt1].answer[cnt2] == '\t') {
    answer_and_question_s[cnt1].answer[cnt2] = '\0';
    break;
   }

   if (feof(fp)) break;

  }

  for (cnt2 = 0; answer_and_question_s[cnt1].question[cnt2] != '\n'; cnt2++) {
   answer_and_question_s[cnt1].question[cnt2] = getc(fp);
   
   if (answer_and_question_s[cnt1].question[cnt2] == '\n') {
    answer_and_question_s[cnt1].question[cnt2] = '\0'; 
    question_max++;
    break;
   }

    if (feof(fp)) break;

  }

 }

 fclose(fp);
 return question_max;
};



int main(int argc, char **argv) {
 DIR *files_dir;
 FILE *reading_fp;
 struct dirent *files_dp;
 struct FilelistStruct filelist_s[FILES_MAX];
 struct AnswerAndQuestion answer_and_question_s[QUESTION_MAX],
        answer_and_question_tmp;

 char files_dir_path[PATH_MAX], user_input_y_or_n,
      user_input_answer[STRINGS_MAX],
      command_line_str[strlen(EDITOR) + NAME_MAX];

 unsigned char cnt = 0, cnt1 = 0,
               number_of_files = 0,
               user_input_num = 0;


 unsigned int question_max = 0;

 XTextProperty win_name;

 XColor blue, exect;
 Colormap cmap;


/* font setting */
 Font font1, font2; /* フォントセット */
 int missing_count; /* 存在しない文字集合の数 */
 char** missing_list; /* 存在しない文字集合 */
 char* def_string; /* ↑に対して描画される文字列 */

/* key event */
 Status status;
 XEvent event;
 KeySym key_sym;
 XIC ic; /* input context */
 char buffer[COL * 2];
 int t_cnt; // XmbLookupString の戻り値

/*文言類はメモリ最適のため後で変更する*/
 char *lab_str1 = "XMEMWORD";
 char *lab_str2 = "is implementation by Xlib, X windowsystem of memword.";
 char *moniter_frame_str = "<moniter>";
 char *user_input_str = "<user_input>";
 char *exit_str = "[ESC key press to exit.]";
 char *start_menu_str1 = "==== memword start menu ====";
 char *file_select_menu_str1 =
  "(Please select a file and enter anumerical value)";
 char *file_select_menu_str2 = "出題ファイルを数値で入力してください。";
 char *back_to_mainmenu_str = "[  0] メインメニューに戻る。";
 char *error_check_strings1 =
  "実際の問題の量以上の値か、負の値が入力されました。";
 char *error_check_strings2 =
  "A number greater than the actual number of file was entered.";

/*出題順序選択画面用*/
 char *order_select_str1 =
  "(Please input are questions order and enter anumerical value)";
 char *order_select_str2 = "出題の順番を数値で入力して下さい。";
 char *order_select_str3 = "[  1] 一行目から順番に出題する。";
 char *order_select_str4 = "[  2] ランダムに出題する。";
 char *order_select_str5 = "[  3] 解答の文字数が少ない順に出題する。";
 char *order_select_str6 = "[  4} 解答の文字数が少ない順に出題する。";
 char question_number_str[256];

/*全問出題分岐画面用*/
 char question_max_str[256];
 char select_all_put_question_str1[256] = "出題数 / ";
 char select_all_put_question_str2[256] = "全問出題しますか?";
 char *select_all_put_question_str3 = "(y/n)";
 char *error_select_all_put_question_str1 = "yかnで入力して下さい。";

/*出題数決定画面用*/

/*出題画面*/
 unsigned short number_of_start_question = 0;
 unsigned short number_of_end_question = 0;
 char qput_question_number_str[256] = "question number : ";
 char qput_line_number_str[256] = "line number : ";
 char qput_question_str[1024] = "Q: ";
 char cnt_str1[256];
 char line_number_str[256];
 char qput_str1[1024];

/*    method        */
 strcat(command_line_str, EDITOR);

 if (NULL == getcwd(files_dir_path, PATH_MAX)) {
  printf("カレントディレクトリの取得に失敗しました。 \n");
  exit(ERROR);
 }

 if (PATH_MAX < (strlen(files_dir_path) + strlen(FILES_DIR_NAME))) {
  printf("出題ファイルまでのパスの長さが PATH_MAX(4096) 以上です。");
  exit(ERROR);
 } else {
  strcat(files_dir_path, FILES_DIR_NAME);
 }

 for (;;) {

  if (!(files_dir = opendir(files_dir_path))) {
   printf("%s ディレクトリが存在しません。\n", files_dir_path);
   exit(ERROR);
  }

// 出題ファイルが格納されているディレクトリまで移動(エラー処理書く)
  if ((-1) == chdir(files_dir_path)) {
   printf("%s ディレクトリへの移動に失敗しました。\n", files_dir_path);
   exit(ERROR);
  };

  for (files_dp = readdir(files_dir), cnt = 0;
   files_dp != NULL; files_dp = readdir(files_dir)) {
    /* . と .. は一覧に代入しない */
   if ((!strcmp(files_dp->d_name,".")) || (!strcmp(files_dp->d_name,".."))) {
    continue;
   }
        
   filelist_s[cnt].file_number = cnt + 1;
   strcpy(filelist_s[cnt].file_name, files_dp->d_name);
   cnt++;
  }

  closedir(files_dir);

  if (0 == cnt) {
   printf("出題用のファイルが存在しません。\n%s.\nに出題用のファイルを作成してください。\n", files_dir_path);
   exit(ERROR);
  };

  number_of_files = cnt;

  /* view method */
  memset(user_input_strings, '\0', sizeof(user_input_strings));

  if(setlocale(LC_CTYPE, "") == NULL) {
   printf("Can't set locale\n");
   exit(EXIT_FAILURE);
  }

  if(! XSupportsLocale()) {
   printf("Current locale is not supported\n");
   exit(EXIT_FAILURE);
  }

  if((disp = XOpenDisplay(NULL)) == NULL) {
   fprintf(stderr, "X server に接続できません\n");
   exit(EXIT_FAILURE);
  }

  root = XCreateSimpleWindow(disp, RootWindow(disp, 0), WIDTH, HEIGHT,
         WIDTH, HEIGHT, 1, BlackPixel(disp, 0), WhitePixel(disp, 0));

  win_name.value = (unsigned char*)"XMEMWORD";
  win_name.encoding = XA_STRING;
  win_name.format = 8;
  win_name.nitems = strlen((char*)win_name.value);
  XSetWMName(disp, root, &win_name);

  /* GC の作成 */
  gc1  = XCreateGC(disp, root, 0, 0);
  gc2 = XCreateGC(disp, root, 0, 0);
  gc3 = XCreateGC(disp, root, 0, 0);
  XSetBackground(disp, gc2, BlackPixel(disp, 0));
  XSetForeground(disp, gc2, WhitePixel(disp, 0));
  font1 = XLoadFont(disp, "-*-fixed-medium-r-normal--14-*-*-*");
  XSetFont(disp, gc1, font1);
  XSetFont(disp, gc2, font1);
  font2 = XLoadFont(disp, "9x15bold");
  XSetFont(disp, gc3, font2);
  cmap = DefaultColormap(disp, 0);
  XAllocNamedColor(disp, cmap, "blue", &blue, &exect);
  XSetForeground(disp, gc3, blue.pixel);

  /* 日本語フォントセットを生成する */
  ja_fs = XCreateFontSet(disp, "-*-fixed-medium-r-normal--14-*-*-*",
          &missing_list, &missing_count, &def_string);

  if(ja_fs == NULL) {
   printf("Failed to create fontset\n");
   exit(EXIT_FAILURE);
  }

  XFreeStringList(missing_list);

  /* サブウィンドウの作成 */
  lab = XCreateSimpleWindow(disp, root, 10, 5,
        (WIDTH - 20), (HEIGHT - 15), 1, BlackPixel(disp, 0),
        WhitePixel(disp,0));

  moniter_frame = XCreateSimpleWindow(disp, root, 32, 25,
                  (WIDTH - 45), (HEIGHT - 150), 0, BlackPixel(disp, 0),
		                WhitePixel(disp, 0));

  question_moniter = XCreateSimpleWindow(disp, root, 37, 40,
                     WIDTH - 72, HEIGHT - 182, 1, WhitePixel(disp, 0),
		                   BlackPixel(disp, 0));

  user_input_moniter = XCreateSimpleWindow(disp, root, 32,
		      ((HEIGHT - 180) + 50), WIDTH - 60, 100, 0,
		      WhitePixel(disp, 0), BlackPixel(disp, 0));

  /* インプットメソッド作成 */
  if((im = XOpenIM(disp, NULL, NULL, NULL)) == NULL) {
   printf("Couldn't open input method.\n");
   exit(EXIT_FAILURE);
  }

  /* インプットコンテキスト作成 */
  ic = XCreateIC(im,
       XNInputStyle,
       XIMPreeditNothing | XIMStatusNothing,
       XNClientWindow,
       user_input_moniter,
       NULL);

  if(ic == NULL) {
   printf("Could't create input context.\n");
   XCloseIM(im);
   exit(EXIT_FAILURE);
  }


  /* 受け取るイベントをサーバに通知 */
  XSelectInput(disp, root, ExposureMask | KeyPressMask);

  XMapWindow(disp, root);
  XMapSubwindows(disp, root);
  XFlush(disp);

  while (1) {
   XNextEvent(disp, &event);

   if (event.type == Expose) {
    memset(user_input_strings, '\0', sizeof(user_input_strings));

    /* Draw the lab place */
    XDrawString(disp, lab, gc3, 6,  15, lab_str1, strlen(lab_str1));
    XDrawString(disp, lab, gc1, 85, 15, lab_str2, strlen(lab_str2));
    XDrawString(disp, lab, gc1, 10, HEIGHT - 20, exit_str, strlen(exit_str));

    /* Draw the moniter_frame place */
    XDrawString(disp, moniter_frame, gc1, 21, 13, moniter_frame_str,
               strlen(moniter_frame_str));
    XDrawLine(disp, moniter_frame, gc1, 0, 8, 0, 190);
    XDrawLine(disp, moniter_frame, gc1, 459, 8, 459, 190);
    XDrawLine(disp, moniter_frame, gc1, 0, 8, 21, 8);
    XDrawLine(disp, moniter_frame, gc1, 83, 8, 459, 8);
    XDrawLine(disp, moniter_frame, gc1, 0, 190, 459, 190);
                    
    /* Draw the moniter place */
    XDrawString(disp, question_moniter, gc2, 3, 13, start_menu_str1,
               strlen(start_menu_str1));

    /* Draw the user_input place */
    XDrawString(disp, user_input_moniter, gc2, 25, 13, user_input_str,
               strlen(user_input_str));
    XDrawString(disp, user_input_moniter, gc2, 8, 28, ":", 1);
    XDrawLine(disp, user_input_moniter, gc2, 4, 8, 4, 93); 
    XDrawLine(disp, user_input_moniter, gc2, 455, 8, 455, 93);
    XDrawLine(disp, user_input_moniter, gc2, 4, 8, 25, 8);
    XDrawLine(disp, user_input_moniter, gc2, 106, 8, 455, 8);
    XDrawLine(disp, user_input_moniter, gc2, 4, 93, 455, 93);

    /* Draw the jp strings on the moniter place */
    XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 30, 
                 start_menu_str2, strlen(start_menu_str2));
    XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 50,
                 start_menu_select1, strlen(start_menu_select1));
    XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 70,
                 start_menu_select2, strlen(start_menu_select2));
    XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 90,
                 start_menu_select3, strlen(start_menu_select3));

    XFlush(disp);
   }

   if (event.type == KeyPress) { // 最初のキーイベント 入力待受
    XLookupString((XKeyEvent *)&event, NULL, sizeof(key_sym), &key_sym, NULL);

    if(key_sym == XK_Escape){
     ExitProgram();
    }

    t_cnt = XmbLookupString(ic, (XKeyPressedEvent*)&event, // キーシムと文字列の両方を返している
    buffer, sizeof(buffer), &key_sym, &status);

    if(key_sym == XK_Delete || key_sym == XK_BackSpace) {
     DeleteCharacter();
     printf("w1_D_%s\n", user_input_strings);
    } else if ((status == XLookupChars || status == XLookupBoth) &&
              !(key_sym == XK_Return)) {
     XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
		  (input_position + 16), 28, buffer, t_cnt);
     input_position += 7;
     user_input_strings[char_cnt] = key_sym;
     char_cnt++;
    } else if (key_sym == XK_Return) {
     UserInputMoniterClear();

//エラーチェック書くこと
     if((user_input_strings[0] == '0') && (user_input_strings[1] == '\0')) {    // 0 プログラム終了押下
      ExitProgram();
     } else if((user_input_strings[0] == '1') &&
              (user_input_strings[1] == '\0')) { // 1 暗記を始める押下

/*出題ファイル選択画面 start*/

    /* 画面クリア */ 
      memset(user_input_strings, '\0', sizeof(user_input_strings)); // ユーザ入力文字格納変数初期化
      ClearQuestionMoniter();

    // 出題ファイル画面文字列出力処理 //
      XDrawString(disp, question_moniter, gc2, 3, 13, file_select_menu_str1,
                 strlen(file_select_menu_str1));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 30,
                   file_select_menu_str2, strlen(file_select_menu_str2));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 50,
                   back_to_mainmenu_str, strlen(back_to_mainmenu_str));

    // 出題ファイル名出力処理 //
      for (cnt = 0; cnt < number_of_files; cnt++) {
       sprintf(question_number_str, "[%3d]", (cnt + 1));
       XmbDrawString(disp, question_moniter, ja_fs, gc2, 3,
                    (70 + (cnt * 20)), question_number_str, 5);
       XmbDrawString(disp, question_moniter, ja_fs, gc2, 45,
                    (70 + (cnt * 20)), filelist_s[cnt].file_name,
                    strlen(filelist_s[cnt].file_name));
      }

      while(1) {
       XNextEvent(disp, &event);
                            
       if (event.type ==  KeyPress) { // 出題ファイル選択画面　入力待受
        printf("while-1_KeyPress\n");
        t_cnt = XmbLookupString(ic, (XKeyPressedEvent*)&event, // キーシムと文字列の両方を返している
                buffer, sizeof(buffer), &key_sym, &status);

        if(key_sym == XK_Escape){
         ExitProgram();
        }

        XLookupString((XKeyEvent *)&event, NULL, sizeof(key_sym), &key_sym,
                     NULL);

        if (key_sym == XK_Delete || key_sym == XK_BackSpace) {
         printf("1_esc_%lu\n", key_sym);
         DeleteCharacter();
        } else if ((status == XLookupChars || status == XLookupBoth) &&
                  !(key_sym == XK_Return)) {
         XClearArea(disp, user_input_moniter, 8, 35, 440, 16, False); // この行とこの下の行は一回の呼び出しにまとめる
         XClearArea(disp, user_input_moniter, 8, 55, 440, 16, False);
         printf("4-%d\n", event.type);
         XmbDrawString(disp, user_input_moniter, ja_fs, gc2, (input_position + 16), 28, buffer, t_cnt);
         input_position += 7;
         user_input_strings[char_cnt] = key_sym;
         char_cnt++;
        } else if (key_sym == XK_Return) {
         UserInputMoniterClear();
         printf("User Pressed Return.\n");
         char_cnt = 0;

     /* 入力エラーチェック */
         if (((user_input_strings[0] - 48) > number_of_files ) &&
            (user_input_strings[1] == '\0')) {
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
		        (input_position + 9), 48, error_check_strings1,
		        strlen(error_check_strings1));
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 9), 68, error_check_strings2,
                       strlen(error_check_strings2));
          input_position = 0;
          user_input_strings[char_cnt] = key_sym;
          char_cnt++;
         } else if ((user_input_strings[0] == '0') &&
                   (user_input_strings[1] == '\0')) { // 0 メインメニューに戻る押下
          printf("0 Pressed. main_meue -> user_file_slect.\n");
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          input_position += 7;
          user_input_strings[char_cnt] = key_sym;
          char_cnt++;
          ClearQuestionMoniter();
          XClearArea(disp, user_input_moniter, 16, 15, 400, 15, False);
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          XDrawString(disp, question_moniter, gc2, 3, 13, start_menu_str1, strlen(start_menu_str1));
          DrawQuestionMoniterJp(Q_SCREEN_1);
          UserInputMoniterClear();
          memset(user_input_strings, '\0', sizeof(user_input_strings));
          break;
         } else if ((user_input_strings[0] >= 1) &&
                   (user_input_strings[1] == '\0')) { // 1以上の数押下   

      /*出題ファイルポインタ取得*/
          if ((reading_fp =
             fopen(filelist_s[(user_input_strings[0] -48) - 1].file_name,
             "r")) == NULL) {
           printf("ファイルの読み込みに失敗しました。\n");
           printf("file open error.\n");
           ExitProgram();
          }

          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          input_position += 7;
          user_input_strings[char_cnt] = key_sym;
          char_cnt++;
          ClearQuestionMoniter();
          XClearArea(disp, user_input_moniter, 16, 15, 400, 15, False);
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          UserInputMoniterClear();
          memset(user_input_strings, '\0', sizeof(user_input_strings));
          break;
         } else { //不必要な分岐かも
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          input_position += 7;
          user_input_strings[char_cnt] = key_sym;
          char_cnt++;
          ClearQuestionMoniter();
          XClearArea(disp, user_input_moniter, 16, 15, 400, 15, False);
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          XDrawString(disp, question_moniter, gc2, 3, 13, start_menu_str1,
                     strlen(start_menu_str1));
                     DrawQuestionMoniterJp(Q_SCREEN_1);
          UserInputMoniterClear();
          memset(user_input_strings, '\0', sizeof(user_input_strings));
          break;
         }

        }

       }
/*出題ファイル選択画面 end*/

      }
//出題順序選択画面 start
      memset(user_input_strings, '\0', sizeof(user_input_strings)); // ユーザ入力文字格納変数初期化
      ClearQuestionMoniter();

//出題順序選択文字列文字列出力
      XDrawString(disp, question_moniter, gc2, 3, 13, order_select_str1,
                 strlen(order_select_str1));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 30,
		    order_select_str2, strlen(order_select_str2));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 50,
		    back_to_mainmenu_str, strlen(back_to_mainmenu_str));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 70,
                   order_select_str3, strlen(order_select_str3));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 90,
		    order_select_str4, strlen(order_select_str4));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 110,
                   order_select_str5, strlen(order_select_str5));
      XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 130,
                   order_select_str6, strlen(order_select_str6));

      while(1) {
       XNextEvent(disp, &event);
                            
       if (event.type ==  KeyPress) { // 出題ファイル選択画面　入力待受
        printf("while-1_KeyPress\n");
        t_cnt = XmbLookupString(ic, (XKeyPressedEvent*)&event, // キーシムと文字列の両方を返している
                buffer, sizeof(buffer), &key_sym, &status);

        if(key_sym == XK_Escape){
         ExitProgram();
        }

        XLookupString((XKeyEvent *)&event, NULL, sizeof(key_sym),
                     &key_sym, NULL);

        if (key_sym == XK_Delete || key_sym == XK_BackSpace) {
         DeleteCharacter();
        } else if ((status == XLookupChars || status == XLookupBoth) &&
                  !(key_sym == XK_Return)) {
         XClearArea(disp, user_input_moniter, 8, 35, 440, 16, False); // この行とこの下の行は一回の呼び出しにまとめる
         XClearArea(disp, user_input_moniter, 8, 55, 440, 16, False);
         printf("4-%d\n", event.type);
         XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                      (input_position + 16), 28, buffer, t_cnt);
         input_position += 7;
         user_input_strings[char_cnt] = key_sym;
         char_cnt++;
        } else if (key_sym == XK_Return) {
         UserInputMoniterClear();
         char_cnt = 0;
        
printf("bbb-%s\n", user_input_strings);
/* 入力エラーチェック */
         if (((user_input_strings[0] - 48) > 6 ) && // 入力された数値が6より大きい
            (user_input_strings[1] == '\0')) {
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 9), 48, error_check_strings1,
                        strlen(error_check_strings1));
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 9), 68, error_check_strings2,
                        strlen(error_check_strings2));
          input_position = 0;
          user_input_strings[char_cnt] = key_sym;
          char_cnt++;
         } else if ((user_input_strings[0] == '0') && // 0 メインメニューに戻る押下
                    (user_input_strings[1] == '\0')) { 
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          input_position += 7;
          user_input_strings[char_cnt] = key_sym;
          char_cnt++;
          ClearQuestionMoniter();
          XClearArea(disp, user_input_moniter, 16, 15, 400, 15, False);
          XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                       (input_position + 16), 28, buffer, t_cnt);
          XDrawString(disp, question_moniter, gc2, 3, 13, start_menu_str1,
                      strlen(start_menu_str1));
          DrawQuestionMoniterJp(Q_SCREEN_1);
          UserInputMoniterClear();
          memset(user_input_strings, '\0', sizeof(user_input_strings));
         } else if (((user_input_strings[0] - 48) == 1) && // 1だった場合の処理
                     (user_input_strings[1] == '\0')) { 
           ClearQuestionMoniter();
// 出題数
// 全問出題しますか?
          question_max = fp_read_and_split(reading_fp, answer_and_question_s);
          memset(question_max_str, '\0', sizeof(question_max_str));
          sprintf(question_max_str, "%d", question_max);
          strcat(select_all_put_question_str1, question_max_str);

          XDrawString(disp, question_moniter, gc2, 3, 13, "(aaa)",
                      strlen("(aaa)"));
          XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 30,
		                      select_all_put_question_str1,
                        strlen(select_all_put_question_str1));
          XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 50,
		                      select_all_put_question_str2,
                        strlen(select_all_put_question_str2));
          XmbDrawString(disp, question_moniter, ja_fs, gc2, 3, 70,
		                      select_all_put_question_str3,
                        strlen(select_all_put_question_str3));

          strcpy(select_all_put_question_str1, "出題数 / ");

printf("ccc-%s\n", user_input_strings);
          memset(user_input_strings, '\0', sizeof(user_input_strings)); // ユーザ入力文字格納変数初期化

          while(1) {
           XNextEvent(disp, &event);
                            
           if (event.type ==  KeyPress) { // 出題ファイル選択画面　入力待受
            printf("while-1_KeyPress\n");
            t_cnt = XmbLookupString(ic, (XKeyPressedEvent*)&event, // キーシムと文字列の両方を返している
                buffer, sizeof(buffer), &key_sym, &status);

            if(key_sym == XK_Escape){
             ExitProgram();
            }

            XLookupString((XKeyEvent *)&event, NULL, sizeof(key_sym),
                           &key_sym, NULL);

           if (key_sym == XK_Delete || key_sym == XK_BackSpace) {
               DeleteCharacter();
           } else if ((status == XLookupChars || status == XLookupBoth) &&
                     !(key_sym == XK_Return)) {
             XClearArea(disp, user_input_moniter, 8, 35, 440, 16, False); // この行とこの下の行は一回の呼び出しにまとめる
             XClearArea(disp, user_input_moniter, 8, 55, 440, 16, False);
             printf("4-%d\n", event.type);
             XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                          (input_position + 16), 28, buffer, t_cnt);
             input_position += 7;
             user_input_strings[char_cnt] = key_sym;
             char_cnt++;
          // } else if (key_sym == XK_Return) {
           //  UserInputMoniterClear();
            // break;
           } else if (key_sym == XK_Return) {
printf("aaa-%s\n", user_input_strings);
             UserInputMoniterClear();
             char_cnt = 0;
/* 入力エラーチェック */
             if ((user_input_strings[0] != 'y') && // 入力された文字がyかn以外
																	(user_input_strings[0] != 'n')) {
                //  (user_input_strings[1] == '\0')) 
                 memset(user_input_strings, '\0',
                 sizeof(user_input_strings)); // ユーザ入力文字格納変数初期化
                  
              XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                           (input_position + 9), 48, error_select_all_put_question_str1,
                            strlen(error_select_all_put_question_str1));
              XmbDrawString(disp, user_input_moniter, ja_fs, gc2,
                           (input_position + 9), 68, error_check_strings2,
                            strlen(error_check_strings2));
              input_position = 0;
              user_input_strings[char_cnt] = key_sym;
              char_cnt++;
             } else if ((user_input_strings[0] == 'y') && // 全問出題選択画面y押下
                        (user_input_strings[1] == '\0')) {
               memset(user_input_strings, '\0',
               sizeof(user_input_strings)); // ユーザ入力文字格納変数初期化
               number_of_end_question = question_max;
        //       ClearQuestionMoniter();


printf("number_of_start_question is %d\n", number_of_start_question);
printf("number_of_end_question is %d\n", number_of_end_question);

/*全問出題開始*/
                         for (cnt = number_of_start_question,
                              cnt1 = 1;
                              cnt < number_of_end_question;
                              cnt++, cnt1++) {
                         sprintf(cnt_str1, "%d    ", cnt1);
                         sprintf(line_number_str, "%ld", answer_and_question_s[cnt].number);
                         strcat(qput_question_number_str, cnt_str1);
																									strcat(qput_line_number_str, line_number_str);
                         strcat(qput_question_str, answer_and_question_s[cnt].question);
                         sprintf(qput_str1, "%s%s", qput_question_number_str, qput_line_number_str);
																						

                         ClearQuestionMoniter();
                         XDrawString(disp, question_moniter, gc2, 3, 13, qput_str1,
                                     strlen(qput_str1));
                          
printf("%s%s\n", qput_question_number_str, qput_line_number_str);
printf("%s\n", qput_str1);
printf("%s\n", qput_question_str);

                         strcpy(qput_question_number_str, "question number : ");
                         strcpy(qput_line_number_str, "line number : ");
                         strcpy(qput_question_str, "Q : ");
                         memset(qput_str1, '\0', sizeof(qput_str1));
                         }

             } else if ((user_input_strings[0] == 'n') &&
                        (user_input_strings[1] == '\0')) {
                         memset(user_input_strings, '\0',
                               sizeof(user_input_strings)); // ユーザ入力文字格納変数初期化
                         ClearQuestionMoniter();
printf("bbbb\n");
ExitProgram();
             }
            }
           }
          }
         }
        }
       }
      }  
   //   memset(user_input_strings, '\0', sizeof(user_input_strings));
     }
    }
   }
  }
  printf("MainExit\n");
  ExitProgram();
//    exit(EXIT_SUCCESS);
 }
}

