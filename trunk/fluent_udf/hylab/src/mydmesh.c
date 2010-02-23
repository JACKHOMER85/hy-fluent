#include "udf.h"
#include "dynamesh_tools.h"

static real v_prev= 0.0;

DEFINE_CG_MOTION(piston, dt, vel, omega, time, dtime)
{
    Thread *t;
    face_t f;
    real NV_VEC (A);
    real force, dv;
    /* reset velocities */
    NV_S (vel, =, 0.0);
    NV_S (omega, =, 0.0);
    if (!Data_Valid_P ())
    return;
    /* get the thread pointer for which this motion is defined */
    t = DT_THREAD ((Dynamic_Thread *)dt);
    /* compute pressure force on body by looping through all faces */
    force = 0.0;
    begin_f_loop (f, t)
    {
        F_AREA (A, f, t);
        force += F_P (f, t) * NV_MAG (A);
    }
    end_f_loop (f, t)
    /* compute change in velocity, i.e., dv= F * dt/ mass;
       velocity update using explicit eulerformula */
    dv= dtime* force / 50.0;
    v_prev+= dv;
    CX_Message ("time = %f, x_vel= %f, force = %f\n", time, v_prev, force);
    /* set x-component of velocity */
    vel[0] = v_prev;
}


/*                                        */
static real G = 9.80; /* ����*/
static real valve_M = 0.1; /* ����*/
static real valve_S = 0.0; /* λ��*/
static real valve_V = 0.0; /* �ٶ�*/
static real valve_F = 0.0; /*����*/
static int is_valve = 0; /* �ж������Ƿ��˶��ķ�ֵ*/
static real ball_S = 0.0; /* λ��*/
static real ball_V = 0.0; /* �ٶ�*/
static real ball_F = 0.0; /*����*/
static int is_ball = 0; /* �ж������Ƿ��˶��ķ�ֵ*/

DEFINE_CG_MOTION(valve, dt, vel, omega, time, dtime)
{
#if !RP_HOST
    Thread *t;
    Domain *d;

    real dv;
    real CG[ND_ND];
    real force[3];
    real moment[3];
    if (!Data_Valid_P())
    {
        Message0("\n\n!!! No data->No mesh motion !!!\n\n");
        return;
    }

    /* */
    NV_S(vel, =, 0.0);
    NV_S(omega, =, 0.0);

    /* force */
    d = THREAD_DOMAIN(DT_THREAD((Dynamic_Thread *)dt));
    t = DT_THREAD(dt);
    NV_S(CG, =, 0.0);
    Compute_Force_And_Moment(d, t, CG, force, moment, FALSE);

    /* force change */
    dv = dtime * (force[0] - G * valve_M) / valve_M;
    if (is_valve == 0 && dv >= 0.0)
    {
        is_valve = 1;
        Message0("\n\n============================== BEGIN VALVE MOTION INFO ==============================\n");
        Message0("\n VALVE BEGIN MOVE! \n");
        Message0("\n============================== END VALVE MOTION INFO ==============================\n");
    }

    if(is_valve == 0 && dv < 0.0)
    {
        dv = 0.0;
    }

    valve_V += dv;
    if(valve_S >= 0.010)
    {
        valve_V = 0.0;
    }

    valve_S += valve_V * dtime;

    /* x velocity */
    vel[0] = valve_V;
    valve_F = force[0];

    Message0("\n\n============================== BEGIN VALVE MOTION INFO ==============================\n");
    Message0("\ntime=%.5e F(x)=%.4e S(x)=%.6e V(x)=%.6e move?=%d\n", time, force[0], valve_S, valve_V, is_valve);
    Message0("\n============================== END VALVE MOTION INFO ==============================\n");
#endif /* RP_HOST */

    node_to_host_real_1(valve_S);
    node_to_host_real_1(valve_V);
    node_to_host_real_1(valve_F);
    node_to_host_real_1(is_valve);

    node_to_host_real(vel, ND_ND);
    node_to_host_real(omega, ND_ND);
}

/*  */
static void write_data(FILE *fp)
{
    fprintf(fp, "%e ", valve_V);
    fprintf(fp, "\n");
    fprintf(fp, "%e ", valve_S);
    fprintf(fp, "\n");
    fprintf(fp, "%e ", valve_F);
    fprintf(fp, "\n");
    fprintf(fp, "%d ", is_valve);
    fprintf(fp, "\n");
}

/* �����ݺ���*/
static void read_data(FILE * fp)
{
    fscanf(fp, "%e", &valve_V);
    fscanf(fp, "%e", &valve_S);
    fscanf(fp, "%e", &valve_F);
    fscanf(fp, "%d", &is_valve);

    fscanf(fp, "%e", &ball_V);
    fscanf(fp, "%e", &ball_S);
    fscanf(fp, "%e", &ball_F);
    fscanf(fp, "%d", &is_ball);
}

DEFINE_RW_FILE(dm_reader, fp)
{
    Message0("\n\n============================== BEGIN UDF INFOMATION ==============================\n");
    Message0("\nReading UDF data from data file...\n");

#if PARALLEL
  #if RP_HOST
    read_data(fp);
  #endif
#else
    read_data(fp);
#endif

    host_to_node_real_1(valve_V);
    host_to_node_real_1(valve_S);
    host_to_node_real_1(valve_F);
    host_to_node_int_1(is_valve);

    host_to_node_real_1(ball_V);
    host_to_node_real_1(ball_S);
    host_to_node_real_1(ball_F);
    host_to_node_int_1(is_ball);

    Message0("\n------------------------------ BEGIN VALVE MOTION INFO ------------------------------\n");
    Message0("\nS(x)=%.6e V(x)=%.6e F(x)=%.4e move?=%d\n", valve_S, valve_V, valve_F, is_valve);
    Message0("\n------------------------------ END VALVE MOTION INFO ------------------------------\n");

    Message0("\n------------------------------ BEGIN BALL MOTION INFO ------------------------------\n");
    Message0("\nS(x)=%.6e V(x)=%.6e F(x)=%.4e move?=%d\n", ball_S, ball_V, ball_F, is_ball);
    Message0("\n------------------------------- END BALL MOTION INFO -------------------------------\n");
    Message0("\n================================ END UDF INFOMATION ================================\n\n");
}

DEFINE_RW_FILE(dm_writer, fp)
{
    Message0("\n\n============================== BEGIN UDF INFOMATION ==============================\n");
    Message0("\n\nWriting UDF data to data file...\n");

#if PARALLEL
#if RP_HOST
    write_data(fp);
#endif
#else
    write_data(fp);
#endif
    Message0("\n================================ END UDF INFOMATION ================================\n\n");
}
