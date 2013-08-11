#include <stdio.h>
#include <math.h>
#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif
#define true 1
#define false 0
typedef int bool;
typedef struct
{
bool collides;
bool state;
float y;
float dy;
char* string;
SDL_Surface *text;
SDL_Rect text_rect;
} Sprite;

typedef struct
{
char numlatin;
char numenglish;
char **latin;
char **english;
} Word;


    // ******************************************Global Variables****************************************/
    //***************************************************************************************************/
    //*************************************You can see them everywhere***********************************/
    //************************ number of lines in file***************************************************************************/
    SDL_Surface* screen;
    SDL_Surface* option;
    SDL_Surface* option2;
    SDL_Surface* background;
    SDL_Surface* questiontext;
    SDL_Rect cur_rect;
    SDL_Rect cur_text_rect;
    SDL_Rect screen_rect;
    TTF_Font *gamefont;
    TTF_Font *gamefontsmall;
    SDL_Color textColor;
    Sprite options[7];
    int optnum=0;
    int milliseconds=0;
    int gamestate=0;
    int questions=0;
    int score;
    char* question=NULL;
    char* answer=NULL;
    bool done = false;
    Word* Words;
    int numwords;
    int usedquestions[25];
    int questionnum=0;
    int usedwords[6];
    int usednum=0;
    // *********************************************Routines*********************************************/
    //***************************************************************************************************/
    //*****************************************Same old, Same old****************************************/
    //***************************************************************************************************/

    int GetClickedOption(int x,int y)
    {
    if(x>(480-200)/2&&x<((480-200)/2)+200)
    {
    int i;
    Sprite curopt;
    for(i=0;i<optnum;i++)
    {
    curopt=options[i];
    if(curopt.y<y&&curopt.y+80>y)
    {
    return i;
    }
    }
    }
    return -1;
    }
    void resetOptions()
    {
    optnum=0;
    }
    void removeOption(int opt)
    {
    Sprite curopt=options[opt];
    int i;
    for(i=opt;i<optnum-1;i++)
    {
    options[i]=options[i+1];
    }
    options[optnum-1]=curopt;
    options[optnum-1].collides=false;
    }


    void createOption(char* text,bool state)
    {
    if(optnum==7)return;
    if(strlen(text)<13)
    {
    options[optnum].text=TTF_RenderText_Blended(gamefont,text,textColor);
    }
    else
    {
    options[optnum].text=TTF_RenderText_Blended(gamefontsmall,text,textColor);
    }
    options[optnum].text_rect.w=options[optnum].text->w;
    options[optnum].text_rect.h=options[optnum].text->h;
    options[optnum].text_rect.x=(480-options[optnum].text_rect.w)/2;
    options[optnum].dy=0;
    options[optnum].y=-80;
    options[optnum].string=text;
    options[optnum].collides=true;
    options[optnum].state=state;
    optnum++;
    }


    void displayOption(Sprite optiontodisplay)
    {
    cur_rect.y=optiontodisplay.y;
    if(optiontodisplay.state)
    {
    SDL_BlitSurface(option2, 0, screen,&cur_rect);
    }
    else
    {
    SDL_BlitSurface(option, 0, screen,&cur_rect);
    }
    cur_text_rect.y=optiontodisplay.y+25;
    cur_text_rect.x=optiontodisplay.text_rect.x;
    cur_text_rect.w=optiontodisplay.text_rect.w;
    SDL_BlitSurface(optiontodisplay.text, 0, screen,&cur_text_rect);
    }

    Sprite collisionResponse(Sprite collided,int collisionheight)
    {
     collided.y=collisionheight;
    if(collided.dy>6)
    {
    collided.dy*=-0.3;
    }
    else
    collided.dy=0;

    return collided;
    }

    void runPhysics()//Yes, really
    {
    int newtime=SDL_GetTicks();
    float deltaT=newtime-milliseconds;
    milliseconds=newtime;
    int i;
    Sprite curopt;
    int curfloor=optnum*80;
    for(i=0;i<optnum;i++)
    {

    //Calculate gravity
    curopt=options[i];
    curopt.dy+=deltaT/10;
    //calculate delta v
    float dv=(curopt.dy*deltaT)/200;

    //Collision detection
    if(curopt.collides)
    {
    //Check for collision with other objects
    if(dv>0&&i>0&&curopt.y+dv+80>options[i-1].y)
    {
    curopt=collisionResponse(curopt,options[i-1].y-80);
    dv=(curopt.dy*deltaT)/200;
    }
    else if(i==0&&curopt.y+dv>curfloor)//Check for collision with floor
    {
   curopt=collisionResponse(curopt,curfloor);
   dv=(curopt.dy*deltaT)/200;
    }
    //else if (dv<0&&i<optnum-1&&curopt.y+dv<options[i+1].y+80)
    //{
    //curopt=collisionResponse(curopt,options[i-1].y+80);
    //}
    }


    //Apply velocity
    if(curopt.y<640)
    {
    curopt.y+=dv;
    }
    options[i]=curopt;
    }
    }



    Word Read_Word(FILE *file)
    {
    Word readword;
    char Chars[64];//buffer to read lines into
    if(fgets(Chars,64,file)!=NULL)
    {
    int i;
    char numlat=1;
    char numeng=1;
    bool islat=true;
    for(i=0;i<64;i++)
    {
    switch(Chars[i])
    {
    case ',':
    if(islat)numlat++;
    else numeng++;
    break;
    case '/':
    islat=false;
    break;
    case '\n':
    i=64;//Terminate the loop
    break;
    }
    }
    readword.english=malloc(numeng*sizeof(char*));
    readword.latin=malloc(numlat*sizeof(char*));
    readword.numenglish=numeng;
    readword.numlatin=numlat;
    //printf("%d English,%d Latin.\n",numeng,numlat);
    //Dynamically allocate an array in which to store string lengths;
    char* latnums=malloc(numlat);
    char* engnums=malloc(numeng);
    memset(latnums,0,numlat);
    memset(engnums,0,numeng);
    islat=true;
    char wordnum=0;
    //Get length of each word
    for(i=0;i<64;i++)
    {
    switch(Chars[i])
    {
    case ',':
    wordnum++;
    break;
    case '/':
    islat=false;
    wordnum=0;
    break;
    case '\n':
    i=64;//Terminate the loop
    break;
    default:
    if(islat)
    {
    //printf("Char %c from word %d which is latin\n",Chars[i],wordnum);
    latnums[wordnum]++;
    }
    else
    {
    //printf("Char %c from word %d which is english\n",Chars[i],wordnum);
    engnums[wordnum]++;
    }
    break;
    }
    }
    //Allocate memory for strings;
    //printf("Latin Word lengths:\n");
    for(i=0;i<numlat;i++)
    {
    readword.latin[i]=malloc((latnums[i]+1)*sizeof(char*));
    memset(readword.latin[i],0,latnums[i]+1);
    //printf("%d\n",latnums[i]);
    }
    //printf("English Word lengths:\n");
    for(i=0;i<numeng;i++)
    {
    readword.english[i]=malloc((engnums[i]+1)*sizeof(char*));
    memset(readword.english[i],0,engnums[i]+1);
   //printf("%d\n",engnums[i]);
    }


    //End allocation of memory for strings
    //Populate strings with data
    islat=true;
    wordnum=0;
    int letternum=0;

    for(i=0;i<64;i++)
    {
    switch(Chars[i])
    {
    case ',':
    wordnum++;
    letternum=0;
    break;
    case '/':
    islat=false;
    wordnum=0;
    letternum=0;
    break;
    case '\n':
    i=64;//Terminate the loop
    break;
    default:
    if(islat)
    {
    readword.latin[wordnum][letternum]=Chars[i];
    letternum++;
    }
    else
    {
    readword.english[wordnum][letternum]=Chars[i];
    letternum++;
    }
    break;
    }
    }
    }
    else
    {
    readword.english=NULL;
    readword.latin=NULL;
    readword.numenglish=0;
    readword.numlatin=0;
    }
    return readword;
    }


    void Free_Word(Word word)
    {
    int i;
    for(i=0;i<word.numlatin;i++)
    {
    free(word.latin[i]);
    }
    for(i=0;i<word.numenglish;i++)
    {
    free(word.english[i]);
    }
    free(word.latin);
    free(word.english);
    }



    int Load_Wordlist()
    {
    FILE* list=fopen("latinwords.txt","rb");
    if(list==NULL)return -1;
    int numberoflines=0;
    int curchar;
    while ((curchar=fgetc(list))!=EOF)
    {
    numberoflines+=(curchar==10)?1:0;
    }
    fseek(list,0,SEEK_SET);
    printf("%d words detected.\n",numberoflines);
    numwords=numberoflines;
    Words=malloc(sizeof(Word)*numberoflines);
    int i;
    for(i=0;i<numberoflines;i++)
    {
    Words[i]=Read_Word(list);
    if(Words[i].latin==NULL)return -1;
    }
    fclose(list);
    return 0;
    }





    /********************************Here is the actual game processing***********************/
    /********************************Here is the actual game processing***********************/
    /********************************Here is the actual game processing***********************/
    /********************************Here is the actual game processing***********************/
    int getRandomWordIndex()
    {
    bool used=true;
    int result=0;
    int i;
    int j;
    int k;
    while (used)
    {
    double fraction=(double)rand()/(double)RAND_MAX;//TODO-Confirm equal probability
    double num=fraction*(double)numwords;
    result=(int)num;
    used=false;
    for(i=0;i<usednum;i++)
    {
    for(j=0;j<Words[result].numenglish;j++)
    {
    for(k=0;k<Words[usedwords[i]].numenglish;k++)
    {
    if(Words[result].english[j]==Words[usedwords[i]].english[k])
    {
    used=true;
    break;
    }
    }
    }
    }
    }
    usedwords[usednum]=result;
    usednum++;
    return result;
    }
    int getRandomNumber(int max)
    {
    double fraction=(double)rand()/(double)RAND_MAX;
    double num=fraction*max;
    return num;
    }
    int getRandomQuestion()
    {
    int question=0;
    bool used=true;
    int i;
    while(used)
    {
    used=false;
    question=getRandomWordIndex();

    for(i=0;i<questionnum;i++)
    {
    if(usedquestions[i]==question)
    {
    used=true;
    break;
    }
    }
    if(used==true)
    {
    usednum--;
    }
    }
    usedquestions[questionnum]=question;
    questionnum++;
    return question;
    }

    void UpdateQuestionText()
    {
    char storage[13];
    char* questionstring=(char*)storage;
    sprintf(questionstring,"Question: %d",questions);
     questiontext=TTF_RenderText_Blended(gamefont,questionstring,textColor);
    }

    void generateEasyQuestion()
    {
    usednum=0;
    questions++;
    UpdateQuestionText();
    printf("Question %d\n",questions);
    printf("Score: %d\n",score);
    resetOptions();
    //Chose correct word
    int word=getRandomQuestion();
    answer=Words[word].english[0];
    question=Words[word].latin[0];
    int position=getRandomNumber(6);
    int i;
    for(i=0;i<6;i++)
    {
    if(i==position)
    {
    createOption(answer,false);
    }
    else
    {
    createOption(Words[getRandomWordIndex()].english[0],false);
    }
    }
    createOption(question,true);
    }

    void generateMediumQuestion()
    {
    int altWord;
    usednum=0;
    questions++;
    UpdateQuestionText();
    printf("Question %d\n",questions);
    resetOptions();
    //Chose correct word
    int word=getRandomQuestion();
    answer=Words[word].english[getRandomNumber(Words[word].numenglish-1)];
    question=Words[word].latin[getRandomNumber(Words[word].numlatin-1)];
    int position=getRandomNumber(6);
    int i;
    for(i=0;i<6;i++)
    {
    if(i==position)
    {
    createOption(answer,false);
    }
    else
    {
    altWord=getRandomWordIndex();
    createOption(Words[altWord].english[getRandomNumber(Words[altWord].numenglish-1)],false);
    }
    }
    createOption(question,true);
    }




  int SearchForward(int start,char* str,char chartomatch)
  {
    int dist=0;
    bool finished=false;
    int max=strlen(str)-1;
    while(start<max&&!finished)
    {
    dist++;
    start++;
    if(str[start]==chartomatch)
    {
    finished=true;
    }
    }
    return dist;
  }


 float getSimilarity(char* str1,char* str2)
    {
     //printf("Comparing %s to the word %s\n",str1,str2);

    int matched=0;
    int toplen=strlen(str1);
    int botlen=strlen(str2);
    int topdist=0;
    int botdist=0;
    int total=toplen+botlen;
    int top=0;
    int bottom=0;


    while(top<toplen&&bottom<botlen)
    {
    //printf("Looping, top is %d and bottom is %d\n",top,bottom);

    if(str1[top]==str2[bottom])
    {
    //printf("Matched %c at %d with %c at %d\n",str1[top],top,str2[bottom],bottom);
    matched+=2;
    top++;
    bottom++;
    }
    else
    {
   // printf("Checking topdist-searching %s from %d for %c\n",str1,top,str2[bottom]);
    topdist=SearchForward(top,str1,str2[bottom]);
    //printf("Checking botdist-searching %s from %d for %c\n",str2,bottom,str1[top]);
    botdist=SearchForward(bottom,str2,str1[top]);
    //printf("topdist is %d while botdist is %d\n",topdist,botdist);
    if(topdist<botdist&&topdist>0)
    {
    top+=topdist;
    }
    else if (botdist>0)
    {
    bottom+=botdist;
    }
    else
    {
    break;
    }
    }
    }
    //printf("Matched %d out of %d\n",matched,total);
    return (float)matched/(float)total;
    }



 void createSimilarWord(char* comparison)
    {
    int curindex=0;
    int number_loops=0;
    float threshold=0.9;
    bool found=false;
    while(!found)
    {
    curindex=getRandomWordIndex();
    number_loops++;
        if(number_loops>100)//decrease threshold amount every 100 attempts
        {
        threshold-=0.1;
        number_loops=0;
        }

    //printf("new word\n"); //check to see if the word is similar enough
    int i;
    for(i=0;i<Words[curindex].numlatin;i++)
    {
    float similarity=getSimilarity(Words[curindex].latin[i],comparison);
    //printf("%f similarity, %f threshold\n",similarity, threshold);
    if(similarity>threshold)
    {
    found=true;
    break;
    }
    }



    if(found)
    {
    createOption(Words[curindex].english[getRandomNumber(Words[curindex].numenglish)],false);//create the word as an option
    found=true;
    }
    else
    {
    usednum--;//That word is not used;
    }
    }
    }




        void generateHardQuestion()
    {
    //printf("%f",getSimilarity("abcdef","aabcdegf"));


    usednum=0;
    questions++;
    UpdateQuestionText();
    //printf("Question %d\n",questions);
    //printf("Score: %d\n",score);
    resetOptions();
    //Chose correct word
    int word=getRandomQuestion();
    answer=Words[word].english[0];
    question=Words[word].latin[0];

    //Chose alternative(wrong) options
    int position=getRandomNumber(6);
    int i;
    for(i=0;i<6;i++)
    {
    if(i==position)//Place correct answer at random location
    {
    createOption(answer,false);
    }
    else
    {
    createSimilarWord(question);
    }
    }
    createOption(question,true);

    }
    char* displayscore(int score)
    {
    char scorestr[6];
    //last 25 points =50% of score
    float transformscore=100-(sqrt(25-score)*8.945);
    sprintf(scorestr,"%.1f",transformscore);
    return scorestr;
    }

    void showScore()
    {
    questionnum=0;
    resetOptions();
    createOption("Return to menu",false);
    createOption("-",false);
    createOption("100",true);
    createOption("Out of a possible:",false);
    char scorestore[6];
    char* scorestr=(char*)scorestore;
    scorestr=displayscore(score);
    createOption(scorestr,true);
    createOption("You scored:",false);
    createOption("Finished!",false);
    }
    void displayMainMenu()
    {
    resetOptions();
    createOption("Quit",false);
    createOption("-",false);
      createOption("Hard",false);
       createOption("Medium",false);
        createOption("Easy",false);
         createOption("-",false);
        createOption("Latin Game!",true);

    }
    void handleClick(int opt)
    {
    if(gamestate==0)//Main menu
    {
    switch(opt)
    {
    case 0://quit
    done=true;
    break;
    case 2://Hard
    gamestate=3;
    generateHardQuestion();
    break;
    case 3:
    gamestate=2;
    generateMediumQuestion();//Medium
    break;
    case 4://Easy
    gamestate=1;
    generateEasyQuestion();
    break;
    }
    }
    else if(gamestate==1||gamestate==2||gamestate==3)//Easy,Medium,Hard
    {
    if(opt==-1)return;//If no option was clicked, do nothing
    if(options[opt].state==true)return;//TODO-fix problems //If the question was clicked, do nothing
    if(strcmp(answer,options[opt].string)!=0)//If the wrong answer was clicked, remove it and deduct score
    {
    removeOption(opt);
    score--;
    }
    else
    {
    score++; //Increase score and generate next question
    if(gamestate==1)
    {
    generateEasyQuestion();
    }
    else if(gamestate==2)
    {
    generateMediumQuestion();
    }
    else if(gamestate==3)
    {
    generateHardQuestion();
    }
    if(questions==26)
    {
    questions=0;
    gamestate=4;
    showScore();
    }
    }
    }
    else if(gamestate==4)
    {
    if(opt==0)
    {
    score=0;
    gamestate=0;
    displayMainMenu();
    }
    }
    }




int main ( int argc, char** argv )
{
      if(Load_Wordlist()==-1)
    {
     printf( "Unable to load wordlist\n");
    return 1;
    }
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }
    if (TTF_Init()==-1)
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }


    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
   screen = SDL_SetVideoMode(480, 640, 16,SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        return 1;
    }

    // *******************************************Load Resources********************************************/
    //******************************************************************************************************/
    //****************************************That's right, resources***************************************/
    //******************************************************************************************************/
    option=IMG_Load("option.png");
    if (!option)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        return 1;
    }
      option2=IMG_Load("option2.png");
    if (!option)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        return 1;
    }
    background=IMG_Load("background.png");
    if (!background)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        return 1;
    }
   gamefont=TTF_OpenFont("Courier_New.ttf",25);
   gamefontsmall=TTF_OpenFont("Courier_New.ttf",16);
    if (gamefont==NULL||gamefontsmall==NULL)
    {
        printf("Unable to load font: %s\n", SDL_GetError());
        return 1;
    }
    // *************************************Initialising some vars********************************************/
    //********************************************************************************************************/
    cur_rect.w=200;
    cur_rect.h=80;
    cur_rect.x=(480-200)/2;
    cur_text_rect.h=20;
    screen_rect.w=480;
    screen_rect.h=640;
    screen_rect.x=0;
    screen_rect.y=0;
    int i;
    for(i=0;i<6;i++){options[i].text=NULL;}
    textColor.r=0;
    textColor.g=0;
    textColor.b=0;
    milliseconds=SDL_GetTicks();
    srand(time(NULL));
    //**************************************************Main loop LOL*******************************************/
    //**********************************************************************************************************/
    //****************************************************Here we go...*****************************************/
    //**********************************************************************************************************/
    displayMainMenu();

    //***********************************************************************************************************
    //********************************************Event processing***********************************************
    //***********************************************************************************************************

    while (!done)
    {
        // message processing loop
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = true;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
                {
                    // exit if ESCAPE is pressed
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        done = true;
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
            {
            handleClick(GetClickedOption(event.button.x,event.button.y));
            }
            } // end switch
        } // end of message processing
        //***************************************************************************************
        //*****************************Rendering*************************************************
        //***************************************************************************************
  // clear screengetline
        SDL_BlitSurface(background, 0, screen, &screen_rect);
        if(gamestate>=1&&gamestate<=3)
        {
        SDL_BlitSurface(questiontext,0,screen,&screen_rect);
        }
        int i;
        for(i=0;i<optnum;i++)
        {
        displayOption(options[i]);
        }

        //Run the physics
        runPhysics();
        // finally, update the screen :)
        SDL_Flip(screen);
    } // end main loop

    // free loaded bitmap
    SDL_FreeSurface(questiontext);
    SDL_FreeSurface(option2);
    SDL_FreeSurface(option);
    SDL_FreeSurface(background);
    TTF_CloseFont(gamefont);
    TTF_Quit();
    for(i=0;i<numwords;i++)
    {
    Free_Word(Words[i]);
    }
    free(Words);
    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
    }
