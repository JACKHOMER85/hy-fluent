#include "udf.h"

#include "mydialog.h"

/************************************************************************/
/*  EOD functions                                                       */
/************************************************************************/
DEFINE_ON_DEMAND(showMsg)
{
    /*Message("message\n");*/
    showMyMessage("Hello!");
}

DEFINE_ON_DEMAND(myAdd)
{
    Message("1+1");
}

DEFINE_ON_DEMAND(runSchemeProc)
{
    CX_Interpret_String("(%udf-on-demand \"showMsg::libhylab\")");
}


/************************************************************************/
/*   2009.12.26: Sigh!!!! I do not how to use it!                       */
/*   2009.12.26: Aha! Use (%run-udf-apply mode)!                        */
/************************************************************************/
DEFINE_EXECUTE_FROM_GUI(sayHello, libhylab, mode)
{
    if(0 == mode)showMyMessage("Hello!");
    Message("Hello From the C Function\n");
}


/************************************************************************/
/* UDF for integrating turbulent dissipation                            */
/* and printing it to console window                                    */
/************************************************************************/
DEFINE_ADJUST(my_adjust, d)
{
    Thread *t;
    /* Integrate dissipation. */
    real sum_diss=0.;
    cell_t c;

    thread_loop_c(t,d)
    {
        begin_c_loop(c,t)
            sum_diss += C_D(c,t)*
            C_VOLUME(c,t);
        end_c_loop(c,t)
    }

    printf("Volume integral of turbulent dissipation: %g\n", sum_diss);
}

/***********************************************************************/
/* UDF that changes the time step value for a time-dependent solution  */
/***********************************************************************/
DEFINE_DELTAT(mydeltat,d)
{
    real time_step;
    real flow_time = CURRENT_TIME;
    if (flow_time < 0.5)
        time_step = 0.1;
    else
        time_step = 0.2;
    return time_step;
}

/***********************************************************************
UDF for initializing flow field variables
************************************************************************/
DEFINE_INIT(my_init_func,d)
{
    cell_t c;
    Thread *t;
    real xc[ND_ND];

    /* loop over all cell threads in the domain  */
    thread_loop_c(t,d)
    {

        /* loop over all cells  */
        begin_c_loop_all(c,t)
        {
            C_CENTROID(xc,c,t);
            if (sqrt(ND_SUM(pow(xc[0] - 0.5,2.),
                pow(xc[1] - 0.5,2.),
                pow(xc[2] - 0.5,2.))) < 0.25)
                C_T(c,t) = 400.;
            else
                C_T(c,t) = 300.;
        }
        end_c_loop_all(c,t)
    }
}

/***********************************************************************
UDFs that increment a variable, write it to a data file
and read it back in
************************************************************************/
int kount = 0;  /* define global variable kount */

DEFINE_ADJUST(demo_calc,d)
{
    kount++;
    printf("kount = %d\n",kount);
}

DEFINE_RW_FILE(writer,fp)
{
    printf("Writing UDF data to data file...\n");
    fprintf(fp,"%d",kount); /* write out kount to data file */
}

DEFINE_RW_FILE(reader,fp)
{
    printf("Reading UDF data from data file...\n");
    fscanf(fp,"%d",&kount); /* read kount from data file */
}


/***********************************************************************
 UDFs that operating the grid
************************************************************************/
int * g_sgrid = 0;

DEFINE_ON_DEMAND(INIT)
{
    int counter;
    int thread_ID;
    Thread *t;
    int i;
    face_t f;   // index of face of wall
    cell_t c0;  // index of cell which is adjacent to wall. Ҳ����B�ڱ����С�
    thread_ID = 8;  // ������Դ��û�����õ�����boundary condition�����£�������A��Ȼ���  //����ʾ��ID���
    counter=0;
    i=0;

    t = Lookup_Thread(1,thread_ID);//1����domain id��һ�������Ϊ1���˾�õ����A��thread.

    // �������face������
    begin_f_loop(f,t)
    {
        c0 = F_C0(f,t);
        counter ++;
    }
    end_f_loop(f,t)

    g_sgrid = (int *) malloc(counter * sizeof(int));

    // ��ȡ�����ڽ���浥Ԫ��id������a���档
    begin_f_loop(f,t)
    {
        c0 = F_C0(f,t);
        g_sgrid[i] = c0;
        i++;
    }
    end_f_loop(f,t)
}

// Դ��
DEFINE_SOURCE(yourSourceName, c, t, dS, eqn)
{
    real source;
    if(cellTest(c))
    {
        //source = your source; //����������õ�����
        return source;
    }
    return 0;
}

// �жϵ�ǰ��Ԫ�Ƿ������ڽ���浥Ԫ
int cellTest(int dd)
{
    int iResult;
    // code for testing dd, if dd is within array a return 1, otherwise return 0.
    //...
    iResult = 0;

    return iResult;
}
