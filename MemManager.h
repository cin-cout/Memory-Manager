#include <stdio.h>
# include <stdlib.h>
#include <string.h>
#include <time.h>
typedef struct list list;
typedef struct vdnode vdnode;

void goTLB(char,int);
FILE *output;

char TLB_p[10],Page_p[10],Frame_p[10];
int n_pro,n_page,n_frame;

int TLB_t[32][2];
long int TLB_time[32];
int **Process[20];
float Rate[20][4];
int *freeframe_l;
int *frame_m;
int *fordisknum;
list * victim_l;
list * dis_l;
vdnode** sn_g;
int timeset=0;

struct vdnode
{
    int page_n;
    int dis_n;
    int frame_n;
    char process;
    vdnode *prev;
    vdnode *next;
};

struct list
{
    vdnode *head;
    vdnode *tail;
};

list* newlist(void)
{
    list* l = (list *) malloc(sizeof(list));
    l-> head = NULL;
    l -> tail = NULL;
    return l;
}

int isempty(list* l)
{
    if(l->head == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

vdnode* new_node(int pn,int dn,int fn,char proc)
{
    vdnode* vdn = (vdnode *) malloc(sizeof(vdnode));
    vdn -> page_n = pn;
    vdn -> dis_n = dn;
    vdn -> frame_n = fn;
    vdn -> process = proc;
    vdn -> prev = NULL;
    vdn -> next = NULL;
    return vdn;
}

void push(list* l,vdnode* vdn)
{

    if(l->head == NULL && l->tail == NULL)
    {
        l->head = vdn;
        l->tail = vdn;
        vdn -> prev = NULL;
        vdn-> next = NULL;
    }

    else
    {
        l -> tail -> next = vdn;
        vdn -> prev = l->tail;
        l->tail = vdn;
        vdn -> next = NULL;
    }

}

vdnode* dis_search(list* l,char proc,int page)
{

    vdnode* sn;
    sn = l -> head;
    while(sn!=NULL)
    {
        if(sn -> process == proc && sn -> page_n == page)
        {
            return sn;
        }
        sn = sn -> next;
    }

    return NULL;
}

vdnode* FIFOlocal_search(list* l,char proc)
{

    vdnode* sn;
    sn = l -> head;
    while(sn!=NULL)
    {
        if(sn -> process == proc)
        {
            return sn;
        }
        sn = sn -> next;
    }

    return NULL;
}

vdnode* CLOCKglobal_search(list* l)
{

    static vdnode* sn = NULL;

    if(sn == NULL)
    {
        sn = l -> head;
    }

    while(1)
    {

        if(Process[(sn ->process)-65][sn->page_n][1] == 1 )
        {
            Process[(sn ->process)-65][sn->page_n][1] = 0;
        }
        else
        {
            vdnode *nnn;
            nnn = sn;
            sn = sn -> next;
            return nnn ;
        }
        sn = sn -> next;
        if(sn == NULL)
        {
            sn = l -> head;
        }

    }

}

vdnode* CLOCKlocal_search(list* l,char proc)
{
    int p = proc -65;
    if(sn_g[p] == NULL)
    {
        sn_g[p] = l -> head;
    }

    while(1)
    {

        if(sn_g[p] -> process == proc)
        {

            if(Process[(sn_g[p] ->process)-65][sn_g[p]->page_n][1] == 1 )
            {
                Process[(sn_g[p] ->process)-65][sn_g[p]->page_n][1] = 0;
            }
            else
            {

                vdnode *nnn;
                nnn = sn_g[p];
                sn_g[p] = sn_g[p] -> next;
                return nnn;
            }
        }
        sn_g[p] = sn_g[p] -> next;
        if(sn_g[p] == NULL)
        {
            sn_g[p] = l -> head;
        }
    }
}

void list_delete(list * l,vdnode* vdn) //return page_n
{

    if(l->head == NULL)
    {
        return;
    }

    else if(vdn == l -> head && vdn == l -> tail)
    {
        l->head = NULL;
        l->tail = NULL;
    }

    else if(vdn == l -> head)
    {
        l -> head = vdn -> next;
        vdn -> next -> prev = NULL;
    }

    else if(vdn == l -> tail)
    {
        l -> tail = vdn -> prev;
        vdn -> prev -> next = NULL;
    }

    else
    {
        vdn -> next -> prev = vdn -> prev;
        vdn -> prev -> next = vdn -> next;
    }

}

void list_delete_and_push(list * l,vdnode* vdn,vdnode* vdn_i) //return page_n
{

    if(l->head == NULL)
    {
        return;
    }

    else if(vdn == l -> head && vdn == l -> tail)
    {
        l->head = vdn_i;
        l->tail = vdn_i;
        vdn_i ->prev =NULL;
        vdn_i ->next =NULL;
    }

    else if(vdn == l -> head)
    {
        l -> head = vdn_i;
        vdn_i -> prev = NULL;
        vdn -> next -> prev = vdn_i;
        vdn_i -> next = vdn -> next;

    }

    else if(vdn == l -> tail)
    {
        l -> tail = vdn_i;
        vdn_i -> prev = vdn -> prev;
        vdn_i -> next = NULL;
        vdn -> prev -> next = vdn_i;
    }

    else
    {
        vdn -> next -> prev = vdn_i;
        vdn -> prev -> next = vdn_i;
        vdn_i -> prev = vdn -> prev;
        vdn_i -> next = vdn -> next;
    }

}

void showlsit()
{
    int i=1;
    vdnode* sn = victim_l->head;
    while(sn!=NULL)
    {

        printf("%d sn : %c   %d   %d\n",i++,sn->process,sn->page_n,Process[(sn ->process)-65][sn->page_n][1]);
        sn = sn->next;
    }

}

void setpara()
{

    FILE *sys_r  = fopen("sys_config.txt","r");
    output = fopen("trace_output.txt","w");

    fscanf(sys_r,"TLB Replacement Policy: %s\n",TLB_p);
    fscanf(sys_r,"Page Replacement Policy: %s\n",Page_p);
    fscanf(sys_r,"Frame Allocation Policy: %s\n",Frame_p);
    fscanf(sys_r,"Number of Processes: %d\n",&n_pro);
    fscanf(sys_r,"Number of Virtual Page: %d\n",&n_page);
    fscanf(sys_r,"Number of Physical Frame: %d",&n_frame);

    //TLB
    for(int i=0; i<32; i++)
    {
        TLB_t[i][0] = -1;
        TLB_t[i][1] = -1;
    }

    //PAGETABLE
    for(int i=0; i<n_pro; i++)
    {
        Process[i] = (int **) malloc(n_page * sizeof(int *));

        for(int j=0; j<n_page; j++)
        {
            Process[i][j] = (int *) malloc(sizeof(int)*3);

            Process[i][j][0] = -1;
            Process[i][j][1] = 0;
            Process[i][j][2] = 0;
        }
    }

    //FREEFRAMELIST
    freeframe_l = (int *) malloc(n_frame * sizeof(int));

    for(int i=0; i<n_frame; i++)
    {
        freeframe_l[i] = 0;
    }

    frame_m = (int *) malloc(n_frame * sizeof(int));

    for(int i=0; i<n_frame; i++)
    {
        frame_m[i] = -1;
    }

    fordisknum = malloc(n_pro*n_page*sizeof(int));

    for(int i=0; i<n_page*n_pro; i++)
    {
        fordisknum[i] = 0;
    }

    victim_l = newlist();
    dis_l = newlist();

    sn_g = malloc(n_pro*sizeof(vdnode *));
    for(int i=0; i<n_pro; i++)
    {
        sn_g[i]=NULL;
    }


    fclose(sys_r);
}

void LRU(int page,int frame)
{

    int mintim = TLB_time[0], index=0;

    for(int i=0; i<32; i++)
    {
        if(TLB_time[i]<mintim)
        {
            mintim = TLB_time[i];
            index = i;
        }

    }

    TLB_t[index][0] = page;
    TLB_t[index][1] = frame;
    TLB_time[index] = timeset++;

}

void RANDOM(int page,int frame)
{

    srand(time(NULL));
    int index = rand()%32;
    TLB_t[index][0] = page;
    TLB_t[index][1] = frame;

}

void updateTLB(int page,int frame)
{

    for(int i=0; i<32; i++)
    {
        if(TLB_t[i][0] == -1)
        {
            TLB_t[i][0] = page;
            TLB_t[i][1] = frame;
            TLB_time[i] = timeset++;
            return;
        }
    }

    if(strcmp(TLB_p,"LRU")==0)
    {
        LRU(page,frame);
    }

    else if(strcmp(TLB_p,"RANDOM")==0)
    {
        RANDOM(page,frame);
    }

}

void FIFO(char proc,int page)
{

    vdnode* nn;
    vdnode* sn;
    int p = proc - 65;
    if(strcmp(Frame_p,"GLOBAL")==0)
    {

        sn = victim_l -> head;
        list_delete(victim_l,sn);

        for(int i=0; i<n_pro*n_page; i++)
        {
            if(fordisknum[i] == 0)
            {
                sn -> dis_n = i;
                fordisknum[i] = 1;
                break;
            }
        }

        push(dis_l,sn);

        fprintf(output," %d, Evict %d of Process %c to %d,",sn->frame_n,sn->page_n,sn->process,sn -> dis_n);

        if(sn->process == proc)
        {
            for(int i=0; i<32; i++)
            {
                if(TLB_t[i][0] == sn->page_n)
                {
                    TLB_t[i][0] = -1;
                    TLB_t[i][1] = -1;
                    break;
                }
            }
        }

        nn = dis_search(dis_l,proc,page);
        if(nn == NULL)
        {
            fprintf(output," %d<<-1\n",page);
            nn = new_node(page,-1,sn->frame_n,proc);
            push(victim_l,nn);
        }
        else
        {
            fprintf(output," %d<<%d\n",page,nn->dis_n);
            list_delete(dis_l,nn);
            fordisknum[nn -> dis_n] = 0;
            nn -> dis_n = -1;
            nn -> frame_n = sn->frame_n;
            push(victim_l,nn);
        }


        frame_m[sn->frame_n] = page;

        Process[(sn->process-65)][sn->page_n][0] = -1;
        Process[(sn->process-65)][sn->page_n][2] = 0;

        Process[p][page][0] = sn->frame_n;
        Process[p][page][2] = 1;


        updateTLB(page,sn->frame_n);
        goTLB(proc,page);
    }

    else if(strcmp(Frame_p,"LOCAL")==0)
    {

        sn = FIFOlocal_search(victim_l,proc);
        list_delete(victim_l,sn);


        for(int i=0; i<n_pro*n_page; i++)
        {
            if(fordisknum[i] == 0)
            {
                sn -> dis_n = i;
                fordisknum[i] = 1;
                break;
            }
        }

        push(dis_l,sn);

        fprintf(output," %d, Evict %d of Process %c to %d,",sn->frame_n,sn->page_n,sn->process,sn -> dis_n);

        if(sn->process == proc)
        {
            for(int i=0; i<32; i++)
            {
                if(TLB_t[i][0] == sn->page_n)
                {
                    TLB_t[i][0] = -1;
                    TLB_t[i][1] = -1;
                    break;
                }
            }
        }

        nn = dis_search(dis_l,proc,page);
        if(nn == NULL)
        {
            fprintf(output," %d<<-1\n",page);
            nn = new_node(page,-1,sn->frame_n,proc);
            push(victim_l,nn);
        }
        else
        {
            fprintf(output," %d<<%d\n",page,nn->dis_n);
            list_delete(dis_l,nn);
            fordisknum[nn -> dis_n] = 0;
            nn -> dis_n = -1;
            nn -> frame_n = sn->frame_n;
            push(victim_l,nn);
        }


        frame_m[sn->frame_n] = page;

        Process[(sn->process-65)][sn->page_n][0] = -1;
        Process[(sn->process-65)][sn->page_n][2] = 0;

        Process[p][page][0] = sn->frame_n;
        Process[p][page][2] = 1;


        updateTLB(page,sn->frame_n);
        goTLB(proc,page);

    }

}

void CLOCK(char proc,int page)
{

    vdnode* nn;
    vdnode* sn;
    int p = proc - 65;
    if(strcmp(Frame_p,"GLOBAL")==0)
    {

        sn = CLOCKglobal_search(victim_l);

        for(int i=0; i<n_pro*n_page; i++)
        {
            if(fordisknum[i] == 0)
            {
                sn -> dis_n = i;
                fordisknum[i] = 1;
                break;
            }
        }

        fprintf(output," %d, Evict %d of Process %c to %d,",sn->frame_n,sn->page_n,sn->process,sn -> dis_n);

        if(sn->process == proc)
        {
            for(int i=0; i<32; i++)
            {
                if(TLB_t[i][0] == sn->page_n)
                {
                    TLB_t[i][0] = -1;
                    TLB_t[i][1] = -1;
                    break;
                }
            }
        }

        nn = dis_search(dis_l,proc,page);
        if(nn == NULL)
        {
            fprintf(output," %d<<-1\n",page);
            nn = new_node(page,-1,sn->frame_n,proc);
        }
        else
        {
            fprintf(output," %d<<%d\n",page,nn->dis_n);
            list_delete(dis_l,nn);
            fordisknum[nn -> dis_n] = 0;
            nn -> dis_n = -1;
            nn -> frame_n = sn-> frame_n;
        }

        list_delete_and_push(victim_l,sn,nn);
        push(dis_l,sn);

        frame_m[sn->frame_n] = page;

        Process[(sn->process-65)][sn->page_n][0] = -1;
        Process[(sn->process-65)][sn->page_n][2] = 0;

        Process[p][page][0] = sn->frame_n;
        Process[p][page][1] = 1;
        Process[p][page][2] = 1;

        updateTLB(page,sn->frame_n);
        goTLB(proc,page);

    }

    else if(strcmp(Frame_p,"LOCAL")==0)
    {
        sn = CLOCKlocal_search(victim_l,proc);

        for(int i=0; i<n_pro*n_page; i++)
        {
            if(fordisknum[i] == 0)
            {
                sn -> dis_n = i;
                fordisknum[i] = 1;
                break;
            }
        }

        fprintf(output," %d, Evict %d of Process %c to %d,",sn->frame_n,sn->page_n,sn->process,sn -> dis_n);

        if(sn->process == proc)
        {
            for(int i=0; i<32; i++)
            {
                if(TLB_t[i][0] == sn->page_n)
                {
                    TLB_t[i][0] = -1;
                    TLB_t[i][1] = -1;
                    break;
                }
            }
        }

        nn = dis_search(dis_l,proc,page);
        if(nn == NULL)
        {
            fprintf(output," %d<<-1\n",page);
            nn = new_node(page,-1,sn->frame_n,proc);
        }
        else
        {
            fprintf(output," %d<<%d\n",page,nn->dis_n);
            list_delete(dis_l,nn);
            fordisknum[nn -> dis_n] = 0;
            nn -> dis_n = -1;
            nn -> frame_n = sn->frame_n;
        }


        list_delete_and_push(victim_l,sn,nn);
        push(dis_l,sn);

        frame_m[sn->frame_n] = page;

        Process[(sn->process-65)][sn->page_n][0] = -1;
        Process[(sn->process-65)][sn->page_n][2] = 0;

        Process[p][page][0] = sn->frame_n;
        Process[p][page][1] = 1;
        Process[p][page][2] = 1;

        updateTLB(page,sn->frame_n);
        goTLB(proc,page);

    }

}

void gofreeframelist(char proc,int page)
{
    int p = proc - 65;
    vdnode* n;
    //freeframe_list
    for(int i=0; i<n_frame; i++)
    {
        if(freeframe_l[i] == 0)
        {
            freeframe_l[i] = 1;
            frame_m[i] = page;
            Process[p][page][0] = i;
            Process[p][page][2] = 1;
            Process[p][page][1] = 1;
            n = new_node(page,-1,i,proc);
            push(victim_l,n);
            fprintf(output," %d, Evict -1 of Process %c to -1, %d<<-1\n",i,proc,page);
            updateTLB(page,i);
            goTLB(proc,page);
            return;
        }
    }

    if(strcmp(Page_p,"FIFO")==0)
    {
        FIFO(proc,page);
    }

    else if(strcmp(Page_p,"CLOCK")==0)
    {
        CLOCK(proc,page);
    }
}

void gopage(char proc,int page)
{
    int p = proc - 65;

    if(Process[p][page][2] == 1)
    {
        fprintf(output," Page Hit, %d=>%d\n",page,Process[p][page][0]);
        Process[p][page][1] = 1;
        updateTLB(page,Process[p][page][0]);
        goTLB(proc,page);
    }

    else
    {
        fprintf(output," Page Fault,");
        Rate[p][2]+=1;
        gofreeframelist(proc,page);
    }


}

void goTLB(char proc,int page)
{

    static char old_proc;
    Rate[proc-65][3] += 1;

    if(old_proc !=  proc)
    {

        for(int i=0; i<32; i++) //flush
        {
            TLB_t[i][0] = -1;
            TLB_t[i][1] = -1;
        }
    }

    for(int i=0; i<32; i++)
    {
        if(TLB_t[i][0] == page) //hit
        {
            Process[(proc-65)][page][1] = 1;
            old_proc = proc;
            Rate[proc-65][1]+=1;
            fprintf(output,"Process %c, TLB Hit, %d=>%d\n",proc,TLB_t[i][0],TLB_t[i][1]);
            TLB_time[i] = timeset++;
            return;
        }
    }

    //miss
    old_proc = proc;
    fprintf(output,"Process %c, TLB Miss,",proc);
    gopage(proc,page);
}

void getstart()
{

    setpara();
    FILE *trace_r = fopen("trace.txt","r");

    char proc;
    int page;


    while(fscanf(trace_r,"Reference(%c, %d)\n",&proc,&page)!=EOF)
    {
        int p = proc - 65;
        Process[p][page][1] = 1;
        Rate[p][0] += 1;
        //printf("Reference(%c, %d)\n",proc,page);
        goTLB(proc,page);
    }


    FILE *ana  = fopen("analysis.txt","w");
    float a;
    for(int i=0; i<n_pro; i++)
    {
        a = Rate[i][1]/Rate[i][3];
        fprintf(ana,"Process %c, Effective Access Time = %.3f\n",i+65,a*120+(1-a)*220);
        fprintf(ana,"Process %c, Page Fault Rate: %.3f\n",i+65,Rate[i][2]/Rate[i][0]);
    }


    fclose(ana);
    fclose(trace_r);
    fclose(output);
}