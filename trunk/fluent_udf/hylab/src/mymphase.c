#include "udf.h"

#include "sg_mphase.h"  /* for CVOF(C,T)*/

/*==============================================================*/
/* water vaporition */
#define T_SAT 373.15 /* ���������¶�100��*/
#define LAT_HT 1.e3  /* ��������Ǳ��J/Kg*/

DEFINE_SOURCE(liq_src, cell, pri_th, dS, eqn) /* Һ������Դ��UDF */
{
  Thread *mix_th, *sec_th;  /* �������������ָ��*/
  real m_dot_l; /* ����Һ������ת�� kg/(m2.s) */

  mix_th = THREAD_SUPER_THREAD(pri_th);  /* ָ�����������༴Һ���ָ�� */
  sec_th = THREAD_SUB_THREAD(mix_th, 1); /*ָ����������������ָ�룬����Ϊ�ڶ���*/

  if(C_T(cell, mix_th)>=T_SAT)  /* ���Һ�൥Ԫ���¶ȸ��������¶ȣ�Һ�����������������ת��*/
  {
    m_dot_l = -0.1*C_VOF(cell, pri_th)*C_R(cell, pri_th)*fabs(C_T(cell, mix_th) - T_SAT)/T_SAT;

    dS[eqn] = -0.1*C_R(cell, pri_th)*fabs(C_T(cell, mix_th) - T_SAT)/T_SAT; /*  ����Դ�������ת��ƫ�� */
  }

  else
  {
    m_dot_l = 0.1*C_VOF(cell, sec_th)*C_R(cell, sec_th) * fabs(T_SAT-C_T(cell,mix_th))/T_SAT;

    /* ���ָ������Һ��ĵ�Ԫ�¶�С�������¶ȣ�������Һ�������ת�ƣ�Һ���*/
    dS[eqn] = 0.;  /* ������������Һ��ת�ƣ�����Һ�������Դ�������ת�Ƶ�ƫ��Ϊ��*/
  }
  return m_dot_l;
}

DEFINE_SOURCE(vap_src, cell, sec_th, dS, eqn) /* ��������Դ��UDF */

{

  Thread * mix_th, *pri_th;

  real m_dot_v;

  mix_th = THREAD_SUPER_THREAD(sec_th); /* ָ�������ĵڶ��༴�����ָ�� */

  pri_th = THREAD_SUB_THREAD(mix_th, 0); /* ָ�����������Һ���ָ�룬Һ��Ϊ���� */

  if(C_T(cell, mix_th)>=T_SAT) /* ����������Ԫ���¶ȸ��������¶ȣ�Һ�����������������ת�� */

{

    m_dot_v = 0.1*C_VOF(cell, pri_th)*C_R(cell, pri_th)*

                fabs(C_T(cell, mix_th) - T_SAT)/T_SAT;

    dS[eqn] = 0.; /* ������Һ��������ת�ƣ��������������Դ�������Һ�������ת�Ƶ�ƫ��Ϊ�� */

   }

  else {

    m_dot_v = -0.1*C_VOF(cell, sec_th)*C_R(cell, sec_th)*

                fabs(T_SAT-C_T(cell,mix_th))/T_SAT;

                    /* ���ָ�������ĵ�Ԫ�¶�С�������¶ȣ�������Һ�������ת�� */

    dS[eqn] = -0.1*C_R(cell, sec_th)* fabs(C_T(cell, mix_th) - T_SAT)/T_SAT;

                     /* ������������Һ��ת�ƣ��������������Դ������������ת�Ƶ�ƫ����Ϊ�� */

}

  return m_dot_v;

}

DEFINE_SOURCE(enrg_src, cell, mix_th, dS, eqn) /* ���ģ������Դ��UDF */

{

  Thread *pri_th, *sec_th;

  real m_dot;

  pri_th = THREAD_SUB_THREAD(mix_th, 0);/* ָ��������Һ���ָ�� */

  sec_th = THREAD_SUB_THREAD(mix_th, 1); /* ָ�������������ָ�� */

  if(C_T(cell, mix_th)>=T_SAT) /* ���������ĵ�Ԫ�¶ȸ��������¶ȡ�����ת����Һ��������ת�ƣ��� �ȣ�����ת����ǰ�и��� */

{

    m_dot = -0.1*C_VOF(cell, pri_th)*C_R(cell, pri_th)*

                fabs(C_T(cell, mix_th) - T_SAT)/T_SAT;

    dS[eqn] =  -0.1*C_VOF(cell, pri_th)*C_R(cell, pri_th)/T_SAT;

   }

  else {                              /*�෴��������Һ��ת������� */

    m_dot = 0.1*C_VOF(cell, sec_th)*C_R(cell, sec_th)*

                fabs(T_SAT-C_T(cell,mix_th))/T_SAT;

    dS[eqn] = 0.1*C_VOF(cell, sec_th)*C_R(cell, sec_th)/T_SAT;}

  return LAT_HT*m_dot;  /*����Ǳ��������ת������˵�����Դ��W/m3 */

}




int get_species1(Domain *d)
{
    Material *mix = mixture_material(d);
    Material *sp;
    int i;
    int n_species;
    real mw[MAX_SPE_EQNS];
    char *sp_name;

    CX_Message("\nIndex\tName\tMW\n");
    n_species = 0; /* reset species counter */

    /* loop over species in mixture */
    mixture_species_loop(mix, sp, i)
    {
        /* get name of species i */
        sp_name = MIXTURE_SITE_SPECIE_NAME(mix, i);
        /* get species molecular weight */
        mw[i] = MATERIAL_PROP(sp, PROP_mwi);
        /* display species data */
        CX_Message("%d\t%s\t%f\n", i, sp_name, mw[i]);
        /* update species counter */
        ++n_species;
    }
    return n_species;
}

int get_species2(Domain *d)
{
    Material *mix = mixture_material(d);
    Material *sp;
    int i;
    int n_species;
    real hf[MAX_SPE_EQNS];
    char *sp_name;

    CX_Message("\nIndex\tName\th_form\n");
    /* get total number of species in mixture */
    n_species = MIXTURE_NSPECIES(mix);

    /* loop over species from 0 to n_species-1 */
    spe_loop(i, n_species)
    {
        /* get species material pointer */
        sp = MIXTURE_SPECIE(mix, i);
        /* get name of species i */
        sp_name = MIXTURE_SITE_SPECIE_NAME(mix, i);

        /* get species hentalpy of formation */
        hf[i] = MATERIAL_PROP(sp, PROP_hform);
        /* display species data */
        CX_Message("%d\t%s\t%f\n", i, sp_name, hf[i]);
    }
    return n_species;
}

int find_species(Domain *d, char * sp_name)
{
    Material *mix = mixture_material(d);
    int sp_index;

    sp_index = mixture_specie_index(mix, sp_name);

    return sp_index;
}

int get_reactions(Domain *d)
{
    Material *mix = mixture_material(d);
    Reaction *r_list = mix->reaction_list;
    Reaction *r;
    Material *sp;
    int n_reactions;
    int n_r;
    int n_p;
    int i;
    int j;
    int m;
    char *r_name;
    char *sp_name;
    real mw;

    /* loop over reactions in the reaction list */
    n_reactions = 0;
    loop(r, r_list)
    {
        r_name = r->name;
        n_r = r->n_reactants;
        n_p = r->n_products;

        /* display reaction data */
        CX_Message("\nReaction name: %s\n", r_name);
        CX_Message("Reactions:\n");
        for (i=0; i<n_r; ++i)
        {
            sp = r->mat_reactant[i];
            j = r->reactant[i];
            m = r->stoich_reactant[i];
            sp_name = MIXTURE_SITE_SPECIE_NAME(mix, j);
            mw = MATERIAL_PROP(sp, PROP_mwi);
            CX_Message("%d\t%s\t%d\t%f\n", j, sp_name, m, mw);
        }

        CX_Message("Products:\n");
        for (i=0; i<n_p; ++i)
        {
            sp = r->mat_product[i];
            j = r->product[i];
            m = r->stoich_product[i];
            sp_name = MIXTURE_SITE_SPECIE_NAME(mix, j);
            mw = MATERIAL_PROP(sp, PROP_mwi);
            CX_Message("%d\t%s\t%d\t%f\n", j, sp_name, m, mw);
        }

        ++n_reactions;
    }

    return n_reactions;
}

DEFINE_ON_DEMAND(hy_list_species)
{
    Domain *fl_domain = Get_Domain(1);
    int ns;
    int method = RP_Get_Integer("spe/method");

    switch (method)
    {
    case 1:
        ns = get_species1(fl_domain);
        break;
    case 2:
        ns = get_species2(fl_domain);
        break;
    default:
        Error("Unknown method!\n");
        break;
    }
    CX_Message("Total number of species: %d\n", ns);
}

DEFINE_ON_DEMAND(locate_species)
{
    Domain *fl_domain = Get_Domain(1);
    int sp_index;
    char *sp_name = RP_Get_String("spe/name");

    sp_index = find_species(fl_domain, sp_name);

    if (-1 != sp_index)
    {
        CX_Message("%s has index: %d\n", sp_name, sp_index);
    }
    else
    {
        CX_Message("Sorry! Didn't find species %s in mixture!\n", sp_name);
    }
}

DEFINE_ON_DEMAND(hy_list_reactions)
{
    Domain *fl_domain = Get_Domain(1);
    int nr;

    nr = get_reactions(fl_domain);
    CX_Message("Total number of reactions: %d\n", nr);
}


