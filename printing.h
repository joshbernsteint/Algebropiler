#define BLACK 0
#define GREEN 2
#define YEL 3
#define CYAN 6
#define WHITE 7

#define DEFAULTBG -40
#define DEFAULTTXT -30

#define TXT_YEL "\033[33m"
#define TXT_RED "\033[31m"
#define TXT_CYAN "\033[36m"
#define TXT_GREEN "\033[32m"
#define TXT_DEF "\033[0m"

#define CHBG(color) printf("\033[%dm", 40 + color)
#define CHTXT(color) printf("\033[%dm", 30 + color)

#define PRINTWHITEBG(str) printf("\033[%dm\033[%dm%s\033[%dm\033[%dm\n", 40 + WHITE, 30+BLACK, str,40+DEFAULTBG, 30+WHITE)

const char* PERSON[] = {
"$$$$$$&$&&&&&&&&&&&&&&&&&$&&&&&&&&&&&&&&&&&&&$$XXXXXXXXXXXXXXXX",
"$$$$$&&$&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&$$$XXXXXXXX$XX",
"$$$&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&$$XXXXXXXX",
"$&$&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&$$$XXXXXX",
"$&&&&&&&&&&&&&&&&&&&&$$$XXxxxxxxxxXXXX$&&$&&&&&&&&&&&&&&$$XXXXX",
"&&&&&&&&&&&&&&&&&$$Xxxx+++++++++++++++xX$X$X$$&&&&&&&&&&&$$XXXX",
"&&&&&&&&&&&&&&&Xx++++++;;;;;;;;;;;;;++++xxXxxXX$&&&&&&&&&&$$XXX",
"&&&&&&&&&&&&$Xx+++;;;;;;;;;;;;;;::;;;;;;+++++xxx$&&&&&&&&&&$$$X",
"&&&&&&&&&&&$x+++;;;;;;;;:;:::::::::;;;;;;;+++++xX$$&&&&&&&&&&$X",
"&&&&&&&&&&Xx+++;;;;::::::::::::::::::;;;;;;;++++xX$&&&&&&&&&&$$",
"&&$&&&&&&Xx+++;;;;:::::::::::::::::::::;;;;;;+++xxX$$&&&&$&&&&$",
"&$$&&&&&Xxx++;;;;::::::::::::::::::::::;;;;;;+++xxX$$$$&&&&&&&&",
"&&&&&&&$xx+++;;;;:::;;::::;;;:::::::::::;;;;++++xxXXXX$&&&$$&&&",
"&&&&&&&Xxx++;;;;;;;:::::::::::::::::;;;;;;;;;+++++xxX$$$&&&&&&&",
"&&&&&&$Xx++;;;;;;;:::::::::::;;;+xxxxx+++;;++;;++++xX$$$&&&&&&&",
"&&&&&$$Xx++;++++;;;::::::::::::;;+++;++++++++++;;+++xX$$$&&&&&&",
"&&&&&&$XxxxX$XXx+;;;::::::::::::::::::::;;+;;++;;;++xX$$$$&&&&&",
"&&&&&$$xxX$Xx++;;;;;;;;::::::;;;;;:::::;;;;++$$$$$XxxX$$$&&&&&&",
"&&&&$$$XXXx+;;;;;;;;;;;;::;;;;++xxxxxxXxx++++++++++xx$$$$$&&&&&",
"&&&&$$$XXXx++++++;;:.:;;;:;;;;+xxx+:..;++xx++;;;++++xX$$&$&&&&&",
"&&&&&$+$Xx+xxX$x$xXx;::;;:;;;;;+++++;:;;+;;;;;;;;+++xXX$X++++$&",
"&&&&&&$XXXX$Xx+++++;;+;+;::;;;:;;;;;::::;::;:;;;;+++xxXx++;+++&",
"&&&&&&&+$xxx+++++++;+;+;;:;;;;;;:::::::::::::::;;+++xxXx++;;;;$",
"&&&&&&&$xxx+++;+;;;;;;+;;;;;;;;;;::::::::::::;;;;++xxxXxx;:;;;X",
"&&&&&&&$x+;;;;;;::;;;++;;;;;;;;;;::::::::::::;;;;++xxxXx+;::;;&",
"&&&&&&&Xx+;;;;::::;;+++;;:;;;;;;;+;;;:::::::;;;;;++xxxxx;;;;+x&",
"&&&&&&&$x+;;;:::::;+;;+;;::;;;;;;+;;;;:::::;;;;;;;++xxxxx;;;+&&",
"&&&&&&&&X+;;;::::;;+++xx++++x+++x+;;;;;;:::;;;;;;;++xx+x+;;;x&&",
"&&&&&&&&&x+;;;;;;;;++xxx++++;;;;;;;;;;;;:::;;;;;;;+++++xx++X&&$",
"&&&&&&&&&Xx++;;;;;;;++++;;;;;;;;;;;;;;;;;;:::;;;;;+x+xxxx&&&&$$",
"&&&&&&&&&$++++;;;;+++++++++;;;++;;;;;;;;;;:::;;;;;xx++xxX&&&$$$",
"&&&&&&&&&&x+++;;;;;+xxxxxxx++;;;+xxx+;;;;::;:;;;;+x+++xx$&&&$$$",
"&&&&&&&&&&$++;+;;;;+X$xx+;::;;xXxXxXx;;;;:;::;;;+xx++xxxX&&&$$$",
"&&&&&&&&&&&$;+;+;;;;+xXX+x;:;+;;++++;;;;;:;:;;:;+x+++xxXX$&&$$$",
"&&&&&&&&&&&&X;+;;;;;+xxx+;;;;;;;+;;;;;;;;;:;;;;+xxx+++xXX$&&&&$",
"&&&&&&&&&&&&&X;+;;+;++xx++++++;;;;;;;;;;;;;+;;++xxx++xXXxX&&&&$",
"&&&&&&&&&&&&&&$+++;++++++++++;;;;;;;;;;;;;+;;++xxx+++xxxxXX&&&$",
"&&&&&&&&&&&&&&&&x++;++++;;;;;;;;:;;;;;;;++;;++xxx+++xxxxx:;$&&$",
"&&&&&&&&&&&&&&&&&X++++++;;;;;;:::;;;;;;++;;+xxxx+++x++x.::;$&&&",
"&&&&&&&&&&&&&&&&&&$xx+++;;;;;;;:;;;;;++++++xxxx+++++;....:$&&&&",
"&&&&&&&&&&&&&&&&&&&$XXxx++;+;;;;;++++xx++xxxx++++;:.....;$&&&&&",
"&&&&&&&&&&&&&&&&&&&&$XXXxxxx+++++xxxxxxxxxx++++;.......X&&&&&&&",
};

#define PRINT_PERSON for(int i = 0; i < 42; i++) PRINTWHITEBG(PERSON[i])