#define _CRT_SECURE_NO_WARNINGS
#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS)
#include <malloc.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "stdafx.h"
#include "ddz_interface.h"
#include "DebugLog.h"
#include "Pokers.h"



#define CLIP3(v,min,max)\
	((v)<(min)?(min)\
	               : ( (v)>(max)?(max):(v)))

#define HAS_ROCKET(h)  (h->hands[LIT_JOKER] && h->hands[BIG_JOKER] )




//poker operations

void clear_poker(POKERS *h)
{
    for (int i=0; i<MAX_POKER_KIND; i++)
    {
        h->hands[i]  = 0;
    }
    h->end=P3;
    h->begin=P3;
    h->total=0;
}




//ly20140326
void add_one_poker(POKERS* a, int p) // c=a+b
{

    a->hands[p]++;
    a->total++;
    a->end=max(a->end,p);
    a->begin=min(a->begin,p);
    sort_poker(a);
}

void sub_one_poker(POKERS* a, int p ) // c=a-b
{
    a->hands[p]--;
    a->total--;
    sort_poker(a);
}
//ly20140326














//检查本手牌是否最大
//不考虑炸弹
BOOL is_combo_biggest(POKERS *opp, COMBO_OF_POKERS * c , int opp1_num, int opp2_num,int lower)
{
    BOOL res = true;
    static   char BigSNum[8]= {7,9,11,13,15,17,18,18};
    int num = get_combo_number(c);
    //check number
    if (opp1_num <num && opp2_num < num)
        return res;

    switch ( c->type )
    {
    case ROCKET:
    case BOMB:
    case PAIRS_SERIES:
    case THREE_SERIES:
    default:
        break;

    case SINGLE_SERIES:
        if ( opp1_num+opp2_num > BigSNum[c->len -5 ])
        {
            int start = c->low+1;
            int len=c->len;
            int end=min(Pa,opp->end);
            for (int i = start ; i<= (end -len + 1);)
            {
                int j,sum=0;
                for (j=i; j<= end; j++) {
                    if ( opp->hands[j]<1) break;
                    //else  if(opp->hands[j] ==1)
                    //	sum++;
                }
                if ( j >= i+ len )
                {
                    int loop = j-i-len;
                    for ( int k=0; k<=loop ; k++)
                    {
                        sum = 0;
                        for (int m=i+k; m<j ; m++)
                            if (opp->hands[m] ==1)
                                sum++;
                        if (sum<4)
                            res = false;
                    }
                }
                i=j+1;
            }
        }
        break;

    case THREE_TWO:
    case THREE_ONE:
        if (c->len > 1) break;
    case THREE:
        for (int k=c->low+1; k<=min(P2,opp->end); k++)
        {
            if ( opp->hands[k]>=3 ) {
                res=false;
                break;
            }
        }
        break;
    case PAIRS :
        for (int k=c->low+1; k<=min(P2,opp->end); k++)
        {
            if ( opp->hands[k]>=2 ) {
                res=false;
                break;
            }
        }
        break;
    case SINGLE :
        for (int k=c->low+1; k<=opp->end; k++)
        {
            if ( opp->hands[k]>=1 ) {
                res=false;
                break;
            }
        }
        break;
    }
    return res;
}






//ly20140324 add begin
int calc_wang_2( POKERS *h)
{
    int ret=0;
    for(int i=P2; i<=BIG_JOKER; i++)
        ret +=h->hands[i];
    return ret;
}
//ly20140324 add end




//update summary of combos
void update_summary(COMBOS_SUMMARY *s, POKERS*h, POKERS * opp,
                    COMBO_OF_POKERS * c,int total, int opp1,int opp2,
                    int lower)
{
    //s->ctrl.single = calc_controls(h,opp,CONTROL_POKER_NUM);
    s->real_total_num = 0;
    s->biggest_num = 0;
    s->extra_bomb_ctrl =0;
    s->not_biggest_num =0;
    s->combo_with_2_controls = 0;
    for (int k=0; k<total; k++)
    {
        if (c[k].type!=NOTHING)
        {
            s->real_total_num++;
            if ( is_combo_biggest(opp,c+k,opp1,opp2,lower) )
            {
                c[k].control = 1;
                s->biggest[(int)(s->biggest_num++)] = &c[k];
                if (c[k].type==BOMB && c[k].low<lower)
                    s->extra_bomb_ctrl ++;
            }
            else
            {
                c[k].control = 0;
                s->not_biggest[(int)(s->not_biggest_num++)] = &c[k];

            }
            if ( get_control_poker_num_in_combo	(c+k,lower)>=2 )//&& c[k].type!=PAIRS)
                s->combo_with_2_controls ++;
        }
    }

    s->combo_total_num = s->real_total_num - s->biggest_num;
    if(s->combo_with_2_controls>0 )
        s->ctrl.single -= (s->combo_with_2_controls*10);

    int series_type=0;
    if (s->series_num>0)
    {
        for (int k=0; k<10; k++)
            series_type+= s->series_detail[k]!=0;
    }
    s->combo_typenum = 	(s->pairs_num!=0)  +  series_type
                        + (s->singles_num!=0) + (s->three_num!=0) + (s->three_two_num!=0)
                        + (s->three_one_num!=0) + (s->pairs_series_num!=0) + (s->threeseries_num!=0)
                        + (s->threeseries_two_num!=0)+ (s->threeseries_one_num!=0)+(s->four_one_num!=0)
                        +(s->four_two_num!=0)
                        ;
    s->combo_smallest_single = s->singles_num>0?s->singles[0]->low:0;
    PRINTF(LEVEL+1,"\nCombos summary: total %d type %d real_total %d biggest %d\n",s->combo_total_num,s->combo_typenum,s->real_total_num,s->biggest_num)	;

}

void sort_all_combos(COMBO_OF_POKERS * c , int total, COMBOS_SUMMARY * s,
                     POKERS *h,
                     POKERS *opp, int opp1, int opp2, /* for check biggest*/
                     int lower
                    )
{
//   s->real_total_num = total;
//    int total = s->combo_total_num;
    memset(s,0,sizeof(COMBOS_SUMMARY));//todo: optimized
    for (int k=0; k<total; k++)
    {
        switch (c[k].type)
        {
        case ROCKET:
            //s->singles[s->series_num]=&c[k];
            //s->rocket_num++;
            //break;
        case BOMB:
            s->bomb[(int)(s->bomb_num)]=&c[k];
            s->bomb_num++;
            break;
        case SINGLE_SERIES:
            s->series[(int)(s->series_num)]=&c[k];
            s->series_num++;
            s->series_detail[c[k].len -5]++;
            break;
        case PAIRS_SERIES:
            s->pairs_series[(int)(s->pairs_series_num)]=&c[k];
            s->pairs_series_num++;
            break;
        case THREE_SERIES:
            s->threeseries[(int)(s->threeseries_num)]=&c[k];
            s->threeseries_num++;
            break;
        case THREE:
            s->three[(int)(s->three_num)]=&c[k];
            s->three_num++;
            break;
        case THREE_ONE:
            s->three_one[(int)(s->three_one_num)]=&c[k];
            s->three_one_num++;
            break;
        case THREE_TWO:
            s->three_two[(int)(s->three_two_num)]=&c[k];
            s->three_two_num++;
            break;
        case PAIRS :
            s->pairs[(int)(s->pairs_num)]=&c[k];
            s->pairs_num++;
            break;
        case SINGLE :
            s->singles[(int)(s->singles_num)]=&c[k];
            s->singles_num++;
            break;
        case 3311:
        case 333111:
        case 33331111:
        case 531:
            s->threeseries_one[(int)(s->threeseries_one_num++)]=&c[k];
            break;
        case 3322:
        case 333222:
        case 33332222:
            s->threeseries_two[(int)(s->threeseries_two_num++)]=&c[k];
            break;
        case 411:
            s->four_one[(int)(s->four_one_num++)]=&c[k];
            break;
        case 422:
            s->four_two[(int)(s->four_two_num++)]=&c[k];
            break;
        default:
            //nothing++;
            PRINTF_ALWAYS("!!ERR, line %d this type of combo not supported : %d\n",c[k].type,__LINE__);
            break;
        case NOTHING:
            break;
        }
    }

    // refine for three;
    int j,num=s->threeseries_num;
//    int not_pad[j]
    for (j=0; j<num; j++) //todo: spilt a pair..
    {
        unsigned int type1[6]= {0,31,3311,333111,33331111,531};
        unsigned int type2[5]= {0,32,3322,333222,33332222};
        if(  s->threeseries[j]->len <= s->singles_num && s->threeseries[j]->len <= s->pairs_num)
        {
            //todo:refine
            if(  s->singles[s->threeseries[j]->len-1]->low > s->pairs[s->threeseries[j]->len-1]->low+1
                    ||(s->singles[s->threeseries[j]->len-1]->low > s->pairs[s->threeseries[j]->len-1]->low
                       && s->singles[s->threeseries[j]->len-1]->low >=lower) )
            {
                goto _use_pairs;
            }
        }
        // for
        if ( s->threeseries[j]->len <= s->singles_num)
        {
            for (int k=0; k< s->threeseries[j]->len ; k++ )
            {
                s->threeseries[j]->three_desc[k] = s->singles[k]->low;

                s->singles[k]->type =NOTHING;
            }
            s->singles_num-=s->threeseries[j]->len ;
            memmove(s->singles,s->singles+s->threeseries[j]->len ,(s->singles_num)*sizeof(void*));
            s->threeseries_one[(int)(s->threeseries_one_num)]= s->threeseries[j];
            s->threeseries[j]->type = type1[(int)(s->threeseries[j]->len)];
            s->threeseries_one_num ++;
            s->threeseries_num --;
            s->threeseries[j] = NULL;
        }
        else if (s->threeseries[j]->len <= s->pairs_num)
        {
_use_pairs:
            for (int k=0; k< s->threeseries[j]->len ; k++ )
            {
                s->threeseries[j]->three_desc[k] = s->pairs[k]->low;
                s->pairs[k]->type =NOTHING;
            }
            s->pairs_num-=s->threeseries[j]->len ;
            memmove(s->pairs,s->pairs+s->threeseries[j]->len ,(s->pairs_num)*sizeof(void*));
            s->threeseries_two[(int)(s->threeseries_two_num)]= s->threeseries[j];
            s->threeseries[j]->type = type2[(int)(s->threeseries[j]->len)];
            s->threeseries_two_num ++;
            //->biggest[s->biggest_num++]=  s->threeseries_two[j];
            s->threeseries_num --;
            s->threeseries[j] = NULL;
        }
        else if(s->threeseries[j]->len <= s->pairs_num*2 + s->singles_num )
        {
            /*
            if( s->singles_num == 1 && s->threeseries[j]->len ==3 ){
                s->threeseries[j]->three_desc[0] = s->singles[0]->low;
                s->singles[0]->type =NOTHING;
                s->singles_num = 0;
                s->threeseries[j]->three_desc[1] = s->pairs[0]->low;
                s->threeseries[j]->three_desc[2] = s->pairs[0]->low;
                 s->pairs[0]->type =NOTHING;
            s->pairs_num --;
              memmove(s->pairs,s->pairs+1 ,(s->pairs_num)*sizeof(void*));
            }
            */
            //int single=0,pair=0;
            //do not support len ==5;
            for (int k=0; k< s->threeseries[j]->len ;  )
            {
                if(s->singles_num>0 && s->pairs_num>0 )
                {
                    if ( k== (s->threeseries[j]->len -1) ||s->singles[0]->low < s->pairs[0]->low )
                    {
__pad_single:
                        s->threeseries[j]->three_desc[k++] = s->singles[0]->low;
                        s->singles[0]->type =NOTHING;
                        s->singles_num-- ;
                        memmove(s->singles,s->singles+1 ,(s->singles_num)*sizeof(void*));
                    }
                    else
                    {
__pad_single_in_pair:
                        s->threeseries[j]->three_desc[k++] = s->pairs[0]->low;
                        if(k==s->threeseries[j]->len)
                        {
                            s->pairs[0]->type= SINGLE;
                            s->pairs_num --;
                            s->singles_num ++;
                            for(int single=s->singles_num ; single>0; single--)
                                s->singles[single]=s->singles[single-1];
                            s->singles[0]=s->pairs[0];
                            memmove(s->pairs,s->pairs+1 ,(s->pairs_num)*sizeof(void*));
                            break;
                        }
                        s->threeseries[j]->three_desc[k++] = s->pairs[0]->low;
                        s->pairs[0]->type =NOTHING;
                        s->pairs_num-- ;
                        memmove(s->pairs,s->pairs+1 ,(s->pairs_num)*sizeof(void*));
                    }
                }
                else if (s->singles_num>0)
                {
                    goto __pad_single;
                }
                else if(s->pairs_num>0)
                {
                    goto __pad_single_in_pair;
                }

            }
            s->threeseries_one[(int)(s->threeseries_one_num)]= s->threeseries[j];
            s->threeseries[j]->type = type1[(int)(s->threeseries[j]->len)];
            //s->biggest[s->biggest_num++]=  s->threeseries_two[j];
            s->threeseries_one_num ++;
            s->threeseries_num --;
            s->threeseries[j] = NULL;
        }
        //todo refine
        //    break;
    }


    if (s->threeseries_num >0 ) //strange
    {
        int k=0;
        for(int j=0; j<3; j++)
        {
            if(s->threeseries[j]!=NULL)
            {
                s->threeseries[k++]=s->threeseries[j];
            }
        }
        if(k!=s->threeseries_num){
            PRINTF_ALWAYS("ERR: 3311 or 3322\n");
		}
    }



    if(s->singles_num>=s->three_num && s->pairs_num>= s->three_num && s->three_num>0)
    {
        int use_pairs_for_three =0;
        if(  s->singles[s->three_num-1]->low > s->pairs[s->three_num-1]->low+1
                ||(s->singles[s->three_num-1]->low > s->pairs[s->three_num-1]->low
                   && s->singles[s->three_num-1]->low >=Pa) )
        {
            use_pairs_for_three=1;
        }
        /*
                if( (game->player_type==UPFARMER && game->players[CUR_PLAYER]->oppDown_num ==1)
                        ||   	(game->player_type==DOWNFARMER && game->players[CUR_PLAYER]->oppUp_num ==1)
                        || (game->player_type==LORD &&
                            ( game->players[CUR_PLAYER]->oppDown_num ==1
                              || game->players[CUR_PLAYER]->oppUp_num==1 ) )
                  )
                {
                    use_pairs_for_three =0;
                }
        */
        for (j=0; j<s->three_num; j++) //todo: spilt a pair..
        {
            // for
            if ( !use_pairs_for_three)
            {
                s->three_one[j]= s->three[j];
                s->three[j]->type = 31;
                s->three[j]->three_desc[0] = s->singles[j]->low;
                s->three_one_num ++;
                s->singles[j]->type =NOTHING;
            }
            else
            {
                //search single first
                s->three_two[j]= s->three[j];
                s->three[j]->type = 32;
                s->three[j]->three_desc[0] = s->pairs[j]->low;
                s->three_two_num = j-s->singles_num;
                s->pairs[j]->type =NOTHING;
            }
        }

        if ( !use_pairs_for_three )
        {
            memmove(s->singles,s->singles+s->three_num,(s->singles_num-s->three_num)*sizeof(void*));
            s->singles_num =(s->singles_num-s->three_num);
        }
        else
        {
            memmove(s->pairs,s->pairs+s->three_num,(s->pairs_num-s->three_num)*sizeof(void*));
            s->pairs_num -= s->three_num;
        }
        s->three_num = 0;
    }
    else
    {
        for (j=0; j<s->three_num; j++) //todo: spilt a pair..
        {
            // for
            if ( j<s->singles_num)
            {
                s->three_one[j]= s->three[j];
                s->three[j]->type = 31;
                s->three[j]->three_desc[0] = s->singles[j]->low;
                s->three_one_num ++;
                s->singles[j]->type =NOTHING;
            }
            else if (j-s->singles_num < s->pairs_num)
            {
                //search single first
                s->three_two[j-s->singles_num]= s->three[j];
                s->three[j]->type = 32;
                s->three[j]->three_desc[0] = s->pairs[j-s->singles_num]->low;
                s->three_two_num = j-s->singles_num+1;
                s->pairs[j-s->singles_num]->type =NOTHING;
            }
            else //todo refine
                break;
        }



        if (j<s->three_num)
        {
            memmove(s->three,s->three+j,(s->three_num-j)*sizeof(void*));
            s->three_num =(s->three_num-j);
            s->singles_num = 0;
            s->pairs_num = 0;
        }
        else if (s->three_num > 0 )
        {
            if ( s->three_num <= s->singles_num )
            {
                memmove(s->singles,s->singles+s->three_num,(s->singles_num-s->three_num)*sizeof(void*));
                s->singles_num =(s->singles_num-s->three_num);
            }
            else
            {

                int number = s->three_num - s->singles_num ;
                s->singles_num = 0;
                memmove(s->pairs,s->pairs+number,(s->pairs_num-number)*sizeof(void*));
                s->pairs_num -= number;
            }
            s->three_num = 0;
        }

    }

    s->real_total_num = 0;
    s->extra_bomb_ctrl 	= 0;
    s->combo_with_2_controls = 0;
    s->real_total_num = 0;
    s->biggest_num = 0;
    s->extra_bomb_ctrl =0;
    s->not_biggest_num =0;
    s->combo_with_2_controls = 0;
    update_summary(s,h,opp,c,total,opp1,opp2,lower);
}




/*
Get the
*/
int search_combos_in_pokers(POKERS *h , POKERS *opps,
							//output
							COMBO_OF_POKERS * c,
							COMBOS_SUMMARY * s)
{
	//int combo_nums=0;
	COMBO_OF_POKERS *presult,*pNow,*pTmp, comb_tmp[20]; //todo: use alloce
	COMBOS_SUMMARY *cur,*pSummry,*tmp , smry_tmp;

	presult= c;
	pSummry= s;

	pNow=&comb_tmp[0];

	memset(presult,0, sizeof(COMBO_OF_POKERS)*20);
	memset(pNow,0 ,sizeof(COMBO_OF_POKERS)*20);

	cur=&smry_tmp;
	memset(cur,0,sizeof(COMBOS_SUMMARY));
	memset(pSummry,0,sizeof(COMBOS_SUMMARY));

	int num=0,numRes=0;
	POKERS t;

	memmove(&t,h,sizeof(POKERS));
	numRes= pSummry->combo_total_num  = search_general_1( &t,presult,0,0,0,0);
	sort_all_combos(presult,numRes, pSummry,h,opps,17,17, Pa);
	DBG(print_all_combos(presult,numRes));

    memmove(&t,h,sizeof(POKERS));
    num= cur->combo_total_num = search_general_2( &t,pNow,0,0,0,0);
    sort_all_combos(pNow,num, cur,h,opps,17,17, Pa);
    DBG(print_all_combos(pNow,num));

	if ( cmp_summary(cur,pSummry))
	{
		numRes = num;
		pTmp = presult;
		presult=pNow;
		pNow=pTmp;
		tmp=pSummry;
		pSummry= cur;
		cur=tmp;
	}
	memmove(&t,h,sizeof(POKERS));
	//        memset(cur,0,sizeof(COMBOS_SUMMARY));
	num = cur->combo_total_num = search_general_3( &t,pNow,0,0,0,0);
	sort_all_combos(pNow,num,cur,h,opps,17,17, Pa);
	// DBG(print_all_combos(pNow,num));


	if ( cmp_summary(cur,pSummry))
	{
		numRes = num;
		pTmp = presult;
		presult=pNow;
		pNow=pTmp;

		tmp=pSummry;
		pSummry= cur;
		cur=tmp;
	}

	// for (int k=0; k<numRes; k++)
	if( c!=presult)
		memmove(c,presult,numRes*sizeof(COMBO_OF_POKERS));

	sort_all_combos(c,numRes,s,h,opps,17,17,Pa);
	s->ctrl.single = calc_controls( h,opps, CONTROL_POKER_NUM );

	DBG(print_all_combos(c, numRes));
	return numRes;
}

int search_combos_in_pokers_with_laizi(POKERS *h , POKERS *opps,
									   //output
									   COMBO_OF_POKERS * c,
									   COMBOS_SUMMARY * s,int hun)
{
	int ctrl_num = 	CONTROL_POKER_NUM;
	POKERS all;
	clear_poker(&all);
	add_poker(h,opps,&all);
	if(all.total <=20 )
		ctrl_num = 4;
	else if(all.total <=10 )
		ctrl_num = 2;
	//player->lower_control_poker = get_lowest_controls(&all,ctrl_num);
	/*int lower_control_poker =*/ get_lowest_controls(&all,ctrl_num);
	//int single = calc_controls( h,opps, ctrl_num );

	COMBOS_SUMMARY *pcombo_summary, combo_summary;
	pcombo_summary = & combo_summary;
	/*for (int k=0; k<20; k++)
	{
	combos[k].type=NOTHING;
	combos[k].control=0;		
	combos[k].len=0;	
	}*/
	int numRes;
	h->hun = hun;
	//PRINTF(LEVEL,"search_combos_in_pokers_with_laizi hun=%d\n",h->hun);
	if(h->hun!=-1 && HUN_NUMBER(h)>0)
	{
		COMBO_OF_POKERS *presult,*pNow,*pTmp, comb_tmp[20];; 
		COMBOS_SUMMARY *cur,*result,*tmp,smry_tmp;
		presult= c;
		pNow=&comb_tmp[0];

		memset(presult,0, sizeof(COMBO_OF_POKERS)*20);
		memset(pNow,0 ,sizeof(COMBO_OF_POKERS)*20);

		result= s;
		cur=&smry_tmp;
		memset(cur,0,sizeof(COMBOS_SUMMARY));
		memset(result,0,sizeof(COMBOS_SUMMARY));

		//int numRes = search_combos_in_hands(game, h , presult , result , player);
		numRes = search_combos_in_pokers(h , opps , presult , result);
		int number;	

#define CMP_AND_SWAP_SMY()\
	/*print_summary(result);*/\
	/*print_summary(cur);*/\
	if ( cmp_summary(cur,result))\
		{\
		numRes = number;\
		pTmp = presult;\
		presult=pNow;\
		pNow=pTmp;\
		tmp=result;\
		result= cur;\
		cur=tmp;\
		/*print_all_combos(presult, number);*/\
		}

		int ben_hun = 0;
		//set first hun values  
		for( int i1=P3; i1<=P2; i1++ )
		{	    
			SET_HUN_TO_i(h, i1);
			//	    PRINTF(VERBOSE, "==set first hun to %c\n", poker_to_char(i1));
			if(IS_HUN(h, i1)) ben_hun++;	
			if(HUN_NUMBER(h)>ben_hun)
			{
				//set second hun values  
				for( int i2=P3; i2<=P2; i2++ )
				{
					SET_HUN_TO_i(h, i2);
					//  	          PRINTF(VERBOSE, "\t==set second hun to %c\n", poker_to_char(i2));					
					if(IS_HUN(h, i2)) ben_hun++;				  
					if(HUN_NUMBER(h)>ben_hun)
					{
						//set second hun values  
						for( int i3=P3; i3<=P2; i3++ )
						{
							SET_HUN_TO_i(h, i3);				               		       
							//                          PRINTF(VERBOSE, "\t\t==set third hun to %c\n", poker_to_char(i3));					 							
							//number = search_combos_in_hands(game, h , pNow,cur,player);	
							number =search_combos_in_pokers(h , opps , pNow , cur);
							CMP_AND_SWAP_SMY()	
								SET_i_to_HUN(h, i3);
						}             
					}
					else
					{
						//number = search_combos_in_hands(game, h , pNow,cur,player);	
						number =search_combos_in_pokers(h , opps , pNow , cur);
						CMP_AND_SWAP_SMY()			         
					}
					if(IS_HUN(h, i2)) ben_hun--;		
					SET_i_to_HUN(h, i2);
				}
			}
			else
			{
				//number = search_combos_in_hands(game, h , pNow,cur,player);	
				number =search_combos_in_pokers(h , opps , pNow , cur);
				CMP_AND_SWAP_SMY()	       
			}
			if(IS_HUN(h, i1)) ben_hun--;		
			SET_i_to_HUN(h, i1);	
		}

		// if( c!=presult)
		memmove(c,presult,numRes*sizeof(COMBO_OF_POKERS));

		sort_all_combos(c,numRes,s,h,opps,17,17,Pa);  //Pa

		/*	memmove(combos,presult,numRes*sizeof(COMBO_OF_POKERS)); 			
		sort_all_combos(game,player->combos,number,player->summary,player->opps,player->oppDown_num
		,player->oppUp_num,player->lower_control_poker,player);*/


	}
	else 
	{
		//PRINTF(LEVEL,"hunnumber==0   ");
		numRes =  search_combos_in_pokers(h , opps , c , s);   
		//  if( c!=presult)
		//      memmove(c,presult,numRes*sizeof(COMBO_OF_POKERS));				 
		sort_all_combos(c,numRes, s,h,opps,17,17, Pa);
	}
	//s->ctrl.single = calc_controls( h,opps, CONTROL_POKER_NUM );
	//player->summary->ctrl.single = single;  
	//PRINTF(LEVEL,"search_combos_in_pokers_with_laizi\n");
	//print_summary(s);
	//if(level>=VERBOSE)
	//	print_suit(player);

	//return true;
	// DBG(print_all_combos(c, numRes));
	return numRes;
}


#define RAND()  rand()
#define RAND_NUM_FROM_RANGE(a,b)\
	((b)<=(a)?(a):((a)+(RAND()%((b)+1-(a)))))

#define get_prob_1000(a) (((RAND()%1000)<(a))?1:0)
#define get_prob_100(a) (((RAND()%100)<(a))?1:0)

static int INSERT_A_CARD(POKERS *p, int val)
{
	p->hands[val%MAX_POKER_KIND]++;
	return 1;
}


static int get_pos(int *remain,int total)
{
	int sum=0;
	for(int i=0; i<total; i++)
		sum+=remain[i];

	if (sum==0)
		return 0;
	sum =  RAND()%sum;
	for(int i=0; i<total; i++)
	{
		if( sum <remain[i])
			return i;
		else
		{
			sum-=remain[i];
		}
	}
	//should never run here
	return 0;
}

int get_current_ev(int* prob_bomb)
{
	int multi_bomb[8]= {1, // no bombs
		2,// 1 bombs
		4,// 2 bombs
		7,// 3 bombs
		14,// 4 bombs
		40,// 5+ bombs
	};
	int sum=0;
	for(int i=0; i<6; i++)
	{
		sum+=prob_bomb[i]* multi_bomb[i];
	}
	return sum;
}
int get_bomb_multiple(double avg)
{
	int prob_bomb[6]= {455, // no bombs
		398,// 1 bombs
		122,
		21,
		2,
		2// 5+ bombs
	};
	int prob_bomb_2[4]= {1979, // 5 bombs
		12,// 6 bombs
		3, //7 bombs
		0
	};
	avg=CLIP3(avg,1.0,8.0);
	int target= 1000.0 * avg;
	int try_time= 0;
	//int step = 20;
	if(target >6000)
	{
		prob_bomb[0]= 195;
		prob_bomb[1]= 198;
		prob_bomb[2]= 198;
		prob_bomb[3]= 300;
		prob_bomb[4]= 71;
		prob_bomb[5]= 38;
	}
	else if (target >4000)
	{
		prob_bomb[0]= 195;
		prob_bomb[1]= 198;
		prob_bomb[2]= 378;
		prob_bomb[3]= 192;
		prob_bomb[4]= 35;
		prob_bomb[5]= 2;
	}
	int current =  get_current_ev(prob_bomb);
	while ( (current+30)< target && try_time<100) // only change for 200
	{
		try_time++;
		if( current < target )
		{
			if(prob_bomb[0]>=200)
			{
				prob_bomb[0]-= 20;
				prob_bomb[1]+= 10;
				prob_bomb[2]+= 7;
				prob_bomb[3]+= 3;
			}
			else if(prob_bomb[1]>=200)
			{
				prob_bomb[1]-= 10;
				prob_bomb[2]+= 5;
				prob_bomb[3]+= 4;
				prob_bomb[4]+= 1;
			}
			else if(prob_bomb[2]>=200)
			{
				prob_bomb[2]-= 5;
				prob_bomb[3]+= 3;
				prob_bomb[4]+= 1;
				prob_bomb[5]+= 1;
			}
			else if(prob_bomb[3]>=190)
			{
				prob_bomb[3]-= 2;
				prob_bomb[4]+= 1;
				prob_bomb[5]+= 1;
			}
		}
		current= get_current_ev(prob_bomb);
	}
	int bomb_num= get_pos(prob_bomb,6);
	if(bomb_num==5)
		bomb_num+=get_pos(prob_bomb_2,3);
	return 1<<bomb_num;
}
/*
bool GetRandomParams(Request* res,
double avg_bomb_multiple_config, //1-8
int better_seat_config,  //0: 不调整, 1 : 低 2:中 3 :高
int card_tidiness_config,  //0: 不调整, 1 : 低 2:中 3 :高
int base_good_config     //0: 不调整, 1 : 低 2:中 3 :高
)
{
if(res==NULL)
return false;
const int prob_better_seat[5]= {0,10,40,70,100};
const int prob_tidiness[5]= {0,30,50,80,100};
const int prob_base_good[5]= {0,30,50,80,100};
better_seat_config = CLIP3(better_seat_config,0,4);
card_tidiness_config = CLIP3(card_tidiness_config,0,4);
base_good_config = CLIP3(base_good_config,0,4);
int prob_better =  prob_better_seat[better_seat_config];
int prob_tide =  prob_tidiness[card_tidiness_config];
int prob_base =  prob_base_good[base_good_config];
res->base_good = get_prob_100(prob_base);
res->card_tidiness = get_prob_100(prob_tide);
res->better_seat = get_prob_100(prob_better)?0:3;
res->call_lord_first = 0;
res->bomb_multiple = get_bomb_multiple(avg_bomb_multiple_config);
return true;
}*/

bool GetRandomParams(Request* res,
					 double avg_bomb_multiple_config, //1-8
					 int better_seat_config,  //0: 不调整, 1 : 低 2:中 3 :高
					 int card_tidiness_config,  //0: 不调整, 1 : 低 2:中 3 :高
					 int base_good_config     //0: 不调整, 1 : 低 2:中 3 :高
					 )
{
	if(res==NULL)
		return false;

	res->base_good = get_prob_100(base_good_config);
	res->card_tidiness = get_prob_100(card_tidiness_config);
	res->better_seat = get_prob_100(better_seat_config)?2:3;
	res->call_lord_first = 0;
	res->bomb_multiple = get_bomb_multiple(avg_bomb_multiple_config);
	return true;
}

//verify it
int verify_game(lordGame *g, int inRuleType)
{
	int nDealCardNum = 17;
	if (inRuleType == 3)
	{
		nDealCardNum = 9;
	}
	POKERS all;
	clear_poker(&all);

	for(int i=0; i< 3 ; i++)
		add_poker(&all,&g->bombs[i],&all);
	for(int i=0; i< 4 ; i++)
		add_poker(&all,&g->other_pokers[i],&all);

	for(int i=0; i< 3 ; i++)
	{
		if((g->other_pokers[i].total+g->bombs[i].total)!=nDealCardNum)
		{
			PRINTF_ALWAYS("error in poker %d,total=%d\n",i,(g->other_pokers[i].total+g->bombs[i].total));
			return 0;
		}
	}

	    for(int i=0; i<= BIG_JOKER ; i++)
		{
			if(g->other_pokers[0].hands[i]<0
				||g->other_pokers[1].hands[i]<0
				||g->other_pokers[2].hands[i]<0
		        ||g->other_pokers[3].hands[i]<0
				){
            PRINTF_ALWAYS("error in poker poker=%d,number=%d\n",i,all.hands[i]);
            return 0;
			}
		}

    //check all
	int nStartCard = P3;
	if (inRuleType == 2)
	{
		nStartCard = P5;
	}
	else if (inRuleType == 3)
	{
		nStartCard = P9;
	}
	for (int i=nStartCard; i<= P2; i++)
    {
        if (all.hands[i]!=4) {
            PRINTF_ALWAYS("error in poker poker=%d,number=%d\n",i,all.hands[i]);
            return 0;
        }
    }
    if (all.hands[LIT_JOKER]!=1 || all.hands[BIG_JOKER]!=1 ) {

		PRINTF_ALWAYS("error in poker XD\n");
		return 0;
	}

	return 1;
}



int  CheckTidiness( COMBOS_SUMMARY * s)
{
	int ret=0;
	//print_summary(s)
	ret=s->singles_num+s->pairs_num;
	//PRINTF(LEVEL,"checktidiness s->singles_num=%d,s->pairs_num=%d",s->singles_num,s->pairs_num);
	for (int k=0; k<s->singles_num; k++)
	{
		if(s->singles[k]->low >= Pa)
			ret--;
	}
	for (int k=0; k<s->pairs_num; k++)
	{
		if(s->pairs[k]->low >= Pa)
			ret--;
	}
	return ret;
}

int  GetSwapPoker_laizi(POKERS* p1_all,POKERS* swap, COMBOS_SUMMARY * s,int max_poker)
{
	//POKERS a;
	//clear_poker(&a);
	//PRINTF(LEVEL,"in side GetSwapPoker_laizi s->singles_num=%d,s->pairs_num=%d\n",s->singles_num,s->pairs_num);
	int ret=0;
	for (int k=0; k<s->singles_num; k++)
	{
		if((s->singles[k]->low <= max_poker) && (p1_all->hands[(int)(s->singles[k]->low)]>0))
		{
			INSERT_A_CARD(swap,s->singles[k]->low);
			ret++;
		}
	}
	for (int k=0; k<s->pairs_num; k++)
	{
		if(s->pairs[k]->low <= max_poker) 
		{
			if(p1_all->hands[(int)(s->pairs[k]->low)]>=2)
			{
				INSERT_A_CARD(swap,s->pairs[k]->low);
				INSERT_A_CARD(swap,s->pairs[k]->low);
				ret+=2;
			}
			else if(p1_all->hands[(int)(s->pairs[k]->low)]==1)
			{
				INSERT_A_CARD(swap,s->pairs[k]->low);
				ret++;
			}
		}
	}
	sort_poker(swap);
	return ret; // 
}


int  GetSwapPoker(POKERS* swap, COMBOS_SUMMARY * s,int max_poker)
{
	POKERS a;
	clear_poker(&a);
	int ret=0;
	for (int k=0; k<s->singles_num; k++)
	{
		if(s->singles[k]->low <= max_poker) {
			combo_2_poker(&a,s->singles[k]);
			add_poker(swap,&a,swap);
			ret++;
		}
	}
	for (int k=0; k<s->pairs_num; k++)
	{
		if(s->pairs[k]->low <= max_poker) {
			combo_2_poker(&a,s->pairs[k]);
			add_poker(swap,&a,swap);
			ret++;
			ret++;
		}
	}
	return ret; // sort_poker(swap);
}
static int calc_exchange_poker( POKERS *h,int max_poker)
{
	int ret=0;
	for(int i=P3; i<=max_poker; i++)
		ret +=h->hands[i];
	return ret;
}
int rand_one_exchange_poker(POKERS *all,int max_poker)
{
	int k,retry=calc_exchange_poker(all,max_poker);
	if(retry==0) {
		for(int i=max_poker+1; i<=BIG_JOKER; i++)
			if (all->hands[i]>0)
			{
				all->hands[i]--;
				return i;
			}
			return -1;///error case
	}
	else {
		/*
		while (1)
		{
		k=rand()%(max_poker+1);
		if (all->hands[k]>0)
		{
		all->hands[k]--;
		return k;
		}
		}
		*/
		{
			k= get_pos_char(all->hands,max_poker+1);
			//rand()%MAX_POKER_KIND;
			if (all->hands[k]>0)
			{
				all->hands[k]--;
				return k;
			}
			else
				return -1;
		}
	}
}
int GetRandomPoker(POKERS *all,POKERS* swap, int num,int max_poker)
{
	int p=0,ret=0;
	clear_poker(swap);

	num=min(num,calc_exchange_poker(all,max_poker));

	for (int k=0; k<num; k++)
	{
		p=rand_one_exchange_poker(all,max_poker);
		if(p>=0)
		{
			swap->hands[p]++;
			ret++;
		}
	}
	return ret;
}
//ly06
int GetSwapArray_laizi(POKERS* p1_all,POKERS* swap_p1, COMBOS_SUMMARY * s,POKERS* swap_p2, COMBOS_SUMMARY * s2,POKERS* swap_all)
{

	int num_p1=GetSwapPoker_laizi(p1_all, swap_p1, s, Pk);
	//PRINTF(LEVEL,"\n*****GetSwapArray_laizi print the p1  %d******\n",num_p1);
	//print_summary(s);
	//print_hand_poker_in_line(swap_p1);

	POKERS swap_p;
	clear_poker(&swap_p);

	int num_p2=GetSwapPoker_laizi(swap_all, swap_p2,  s2, Pk);     //p2 swap_p2
	//PRINTF(LEVEL,"\n*****print the p2 poker of single and pairs %d******",num_p2);
	//print_hand_poker_in_line(swap_p2);

	for(int i=Pa; i<MAX_POKER_KIND; i++)
		swap_all->hands[i]=0;

	//PRINTF(LEVEL,"\n*****print the swap all poker <A****");
	//print_hand_poker_in_line(swap_all);

	//swap_all sub singles and pairs <A
	sub_poker(swap_all,swap_p2,swap_all);
	//PRINTF(LEVEL,"\n*****print the swap all poker exclude singes and pairs <A****");
	//print_hand_poker_in_line(swap_all);

	if(num_p2<num_p1)
	{
		int n = calc_exchange_poker(swap_all,Pk);
		if(n>(num_p1-num_p2))
		{
			//Get ramdom pokers to swap_p from swap_all
			int num_p = GetRandomPoker(swap_all,&swap_p,num_p1-num_p2,Pk);

			//PRINTF(LEVEL,"\n*****print GetRandomPoker %d****",num_p);
			//print_hand_poker_in_line(&swap_p);   //swap_p random

			add_poker(&swap_p,swap_p2,swap_p2);

			num_p2 = num_p2+num_p;
		}
		else
		{
			add_poker(swap_all,swap_p2,swap_p2);

			num_p2 = num_p2+n;
		}

		//PRINTF(LEVEL,"\n*****print the P2 %d*****",num_p2);
		//print_hand_poker_in_line(swap_p2);

		if(num_p2<num_p1)
		{
			clear_poker(&swap_p);
			num_p1 = GetRandomPoker(swap_p1,&swap_p,num_p2,Pk);
			clear_poker(swap_p1);
			add_poker(swap_p1,&swap_p,swap_p1);
			//PRINTF(LEVEL,"\n*****print the P1 %d****",num_p1);
			//print_hand_poker_in_line(swap_p1);
		}

	}
	else if(num_p2>num_p1) {
		int num_p = GetRandomPoker(swap_p2,&swap_p,num_p1,Pk);
		num_p2 = num_p;
		clear_poker(swap_p2);
		add_poker(swap_p2,&swap_p,swap_p2);
		//PRINTF(LEVEL,"\n*****print the P2 %d****",num_p2);
		//print_hand_poker_in_line(swap_p2);
	}
	//PRINTF(LEVEL,"\n*****print the P1 %d****",num_p1);
	//print_hand_poker_in_line(swap_p1);
	//PRINTF(LEVEL,"\n*****print the P2 %d*****",num_p2);
	//print_hand_poker_in_line(swap_p2);
	//PRINTF(LEVEL,"\n");
	return num_p1;

}

//ly06
int GetSwapArray(POKERS* swap_p1, COMBOS_SUMMARY * s,POKERS* swap_p2, COMBOS_SUMMARY * s2,POKERS* swap_all)
{

	int num_p1=GetSwapPoker( swap_p1,  s, Pk);
	//PRINTF(LEVEL,"\n*****print the p1  %d******",num_p1);
	//print_hand_poker_in_line(swap_p1);

	POKERS swap_p;
	clear_poker(&swap_p);

	int num_p2=GetSwapPoker( swap_p2,  s2, Pk);     //p2 swap_p2
	//PRINTF(LEVEL,"\n*****print the p2 poker of single and pairs %d******",num_p2);
	// print_hand_poker_in_line(swap_p2);

	for(int i=Pa; i<MAX_POKER_KIND; i++)
		swap_all->hands[i]=0;

	//PRINTF(LEVEL,"\n*****print the swap all poker <A****");
	//print_hand_poker_in_line(swap_all);

	//swap_all sub singles and pairs <A
	sub_poker(swap_all,swap_p2,swap_all);
	//PRINTF(LEVEL,"\n*****print the swap all poker exclude singes and pairs <A****");
	//print_hand_poker_in_line(swap_all);

	if(num_p2<num_p1)
	{
		int n = calc_exchange_poker(swap_all,Pk);
		if(n>(num_p1-num_p2))
		{
			//Get ramdom pokers to swap_p from swap_all
			int num_p = GetRandomPoker(swap_all,&swap_p,num_p1-num_p2,Pk);

			// PRINTF(LEVEL,"\n*****print GetRandomPoker %d****",num_p);
			//print_hand_poker_in_line(&swap_p);   //swap_p random

			add_poker(&swap_p,swap_p2,swap_p2);

			num_p2 = num_p2+num_p;
		}
		else
		{
			add_poker(swap_all,swap_p2,swap_p2);

			num_p2 = num_p2+n;
		}

		//PRINTF(LEVEL,"\n*****print the P2 %d*****",num_p2);
		//print_hand_poker_in_line(swap_p2);

		if(num_p2<num_p1)
		{
			clear_poker(&swap_p);
			num_p1 = GetRandomPoker(swap_p1,&swap_p,num_p2,Pk);
			clear_poker(swap_p1);
			add_poker(swap_p1,&swap_p,swap_p1);
			//PRINTF(LEVEL,"\n*****print the P1 %d****",num_p1);
			//print_hand_poker_in_line(swap_p1);
		}

	}
	else if(num_p2>num_p1) {
		int num_p = GetRandomPoker(swap_p2,&swap_p,num_p1,Pk);
		num_p2 = num_p;
		clear_poker(swap_p2);
		add_poker(swap_p2,&swap_p,swap_p2);
		//PRINTF(LEVEL,"\n*****print the P2 %d****",num_p2);
		//print_hand_poker_in_line(swap_p2);
	}
	//PRINTF(LEVEL,"\n*****print the P1 %d****",num_p1);
	//print_hand_poker_in_line(swap_p1);
	//PRINTF(LEVEL,"\n*****print the P2 %d*****",num_p2);
	//print_hand_poker_in_line(swap_p2);
	//PRINTF(LEVEL,"\n");
	return num_p1;

}

void lookCard(lordGame * game)
{
	glog.log("----------------verify_game start");
	for (int i = 0; i < 4; i++)
	{
		string strBuffBomb;
		string strBuffOther;
		for (int j = 0; j <= P2; j++)
		{
			for (int h = 0; h < game->bombs[i].hands[j]; h++)
			{
				stringstream ss;
				ss << j + 3;
				strBuffBomb += ss.str() + ", ";
			}
			for (int h = 0; h < game->other_pokers[i].hands[j]; h++)
			{
				stringstream ss;
				ss << j + 3;
				strBuffOther += ss.str() + ", ";
			}
		}
		glog.log("ID:%d Bomb:%s Other:%s", i, strBuffBomb.c_str(), strBuffOther.c_str());
	}
	glog.log("----------------verify_game end");
}


//skill34 是否跳过3 4
void  setbomb(lordGame* g, int num, int inRuleType)
{
	static const int prob_rocket[]= {0, 58, 74, 82, 87, 90, 93, 96};
	int probR, remain[4]= {17,17,17,3};
	int bomb_value[13] = { 0 };
	if(num>=8)
		probR = 100;
	else
		probR = prob_rocket[num];

	int is_R= num!=0? get_prob_1000(probR*10) : 0 ;
	if(is_R)
	{
		int pos=RAND_NUM_FROM_RANGE(0,2);
		g->bombs[pos].hands[BIG_JOKER] = 1;
		g->bombs[pos].hands[LIT_JOKER] = 1;
		num--;
		g->all.hands[BIG_JOKER] = 0;
		g->all.hands[LIT_JOKER] = 0;
		remain[pos]-=2;
		g->bomb_number[pos]++;
	}
	
	//init bomb_values
	for(int i=P3; i<=P6; i++){
		bomb_value[i]=5;
	}
	for(int i=P7; i<=P10; i++){
		bomb_value[i]=1;
	}
	for(int i=Pj; i<=P2; i++){
		bomb_value[i]=3;
	}
	if (inRuleType == 2)
	{
		g->other_pokers[2].hands[P3] = 4;
		g->other_pokers[2].hands[P4] = 4;
		for (int i = P3; i < P5; i++)
		{
			g->all.hands[i] = 0;
			bomb_value[i] = 0;
		}
		remain[2] -= 8;
	}
	else if (inRuleType == 3)
	{
		for (int i = P3; i < P9; i++)
		{
			g->all.hands[i] = 0;
			bomb_value[i] = 0;
		}
		remain[0] -= 8;
		remain[1] -= 8;
		remain[2] -= 8;
	}


	//int remain_poker_kind_minus_1 = P2-P3;
	for(int i=num; i>0; i --)
	{
		int val= get_pos(bomb_value, P2+1);	
		bomb_value[val]=0;
__re_find_pos_set_bomb:
		int pos=get_pos(remain,3);
		if(remain[pos]<4)
			goto __re_find_pos_set_bomb;

		g->bombs[pos].hands[val]=4;
		g->all.hands[val]=0;
		remain[pos]-=4;
		g->bomb_number[pos]++;
	}

	if(!is_R)//no rockets
	{
		int pos=get_pos(remain,4),pos1;
		g->other_pokers[pos].hands[BIG_JOKER]=1;
		remain[pos]--;
		do {
			pos1=get_pos(remain,4);
		}
		while(pos1==pos);

		g->other_pokers[pos1].hands[LIT_JOKER]=1;
		remain[pos1]--;
		g->all.hands[BIG_JOKER] = 0;
		g->all.hands[LIT_JOKER] = 0;
	}

	//other cards
	{
		int val=0;
		for(int i=0; i< 4 ; i++)
		{
			int retry=0;
			for(int j=0; j< remain[i] ; j++)
			{
re_get_val:
				val = get_pos_char(g->all.hands,MAX_POKER_KIND);
				if( g->other_pokers[i].hands[val]==3) {
					retry++;
					if(retry<=10)
						goto re_get_val;
				}
				retry=0;
				g->other_pokers[i].hands[val] ++;
				g->all.hands[val]--;
			}
			sort_poker(&g->other_pokers[i]);
			//test functions
			COMBO_OF_POKERS c;
			if(getBomb(&g->other_pokers[i],&c)) {
				g->bomb_number[i]++;
				PRINTF(LEVEL,"WARNING! Extra bomb detected\n");
			}
		}
	}

	for(int i=0; i< 3 ; i++)
		sort_poker(&g->bombs[i]);

}

void reset_game(lordGame* g)
{
	memset (g,0, sizeof(lordGame));
	full_poker(&g->all);
}

void swap_card(lordGame* g,int pos1,int pos2)
{
	POKERS t;
	t= 	g->bombs[pos1] ;
	g->bombs[pos1] = g->bombs[pos2];
	g->bombs[pos2] =t;

	t= 	g->other_pokers[pos1] ;
	g->other_pokers[pos1] = g->other_pokers[pos2];
	g->other_pokers[pos2] =t;

	int num= 	g->bomb_number[pos1] ;
	g->bomb_number[pos1] = g->bomb_number[pos2];
	g->bomb_number[pos2] =num;
}

int GetHun(POKERS* h)
{
	int p[40],num=0;
	for(int i=P3;i<=P2;i++)
	{  
		if(h->hands[i]==1){
			p[num]=i;
			num+=1;
		}	
		if(h->hands[i]==2){
			p[num]=i;
			p[num+1]=i;
			p[num+2]=i;
			p[num+3]=i;
			num+=4;
		}
		if(h->hands[i]==3){
			p[num]=i;
			p[num+1]=i;
			p[num+2]=i;
			num+=3;
		}
		if(h->hands[i]==4){
			p[num]=i;
			p[num+1]=i;
			num+=2;
		}
	}  
	int k=rand()%(num-1);
	return p[k];
}
void  settidiness_laizi(lordGame* g, int tidi_seat, int swap_seat, int first_seat)
{
	int tidiness_num=2;   //ly20140325 add

	POKERS play_card[3],tmp,all,pall,sbomb[3];
	clear_poker(&pall);

	if(g->hun == -1)
	{
		add_poker( &pall,&g->other_pokers[tidi_seat], &pall);
		add_poker( &pall,&g->bombs[tidi_seat], &pall);
		if(tidi_seat==first_seat)
		{
			add_poker(&pall,&g->other_pokers[3],&pall);
		}
		g->hun =  GetHun(&pall);
		PRINTF(LEVEL,"===GetHun, hun=%d ===\n",g->hun);
	}

	clear_poker(&all);
	for (int i=0; i<4; i++)
		add_poker( &all,&g->other_pokers[i], &all);

	for (int i=0; i<3; i++)
	{
		clear_poker(&sbomb[i]);
		play_card[i] = g->other_pokers[i];
		if(i==first_seat)
		{
			add_poker( &g->other_pokers[3],&play_card[i], &play_card[i]);
		}     
		play_card[i].hun=g->hun;

		if(play_card[i].hands[g->hun]>0)   //ly510
		{
			for(int k=0;k<MAX_POKER_KIND;k++)
			{
				if((play_card[i].hands[k]==3)&&( k!=g->hun))
				{
					sbomb[i].hands[k]=3;
					sbomb[i].hands[g->hun]++;
					sbomb[i].total = 4;
					sort_poker(&sbomb[i]); 
					sub_poker(&play_card[i],&sbomb[i],&play_card[i]);

					sub_poker(&all,&sbomb[i],&all);
					sort_poker(&all);
					k=MAX_POKER_KIND;
				}
			}
		}
		sub_poker( &all,&play_card[i], &tmp);
		search_combos_in_pokers_with_laizi(&play_card[i], &tmp,
			g->cs[i], &g->smry[i],g->hun);
		print_summary(&g->smry[i]);
	}

	COMBOS_SUMMARY *s = &g->smry[tidi_seat];
	COMBOS_SUMMARY *s2 = &g->smry[swap_seat];

	//ly20140325 add start

	PRINTF(LEVEL,"===settidiness tidiseat is %d,swap seat is %d,first seat is %d ===\n",tidi_seat,swap_seat,first_seat);

	POKERS swap_p1,swap_p2,swap_all,p1_all;
	int swap_num;
	int retry=15;
	bool c_tidi= false;
	bool c_tidi_0 =false;
	if(CheckTidiness(s) <= tidiness_num) {
		c_tidi = true;
		c_tidi_0 = true;
		PRINTF(LEVEL,"===CheckTidiness succeed0===\n");
	}

	while(retry)
	{
		clear_poker(&swap_p1);
		clear_poker(&swap_p2);
		clear_poker(&swap_all);
		clear_poker(&p1_all);

		//ly06
		//Put all the other pokers to swap_all
		swap_all = play_card[swap_seat];   //g->other_pokers[swap_seat];  //lydebug
		p1_all = play_card[tidi_seat];
		swap_num = GetSwapArray_laizi(&p1_all,&swap_p1,s,&swap_p2,s2,&swap_all);

		//Begin to exchange num_p1 pokers in P1( swap_p1) and P2(swap_p2)
		sub_poker(&play_card[tidi_seat],&swap_p1,&play_card[tidi_seat]);		
		sub_poker(&play_card[swap_seat],&swap_p2,&play_card[swap_seat]);

		add_poker(&play_card[tidi_seat],&swap_p2,&play_card[tidi_seat]);
		add_poker(&play_card[swap_seat],&swap_p1,&play_card[swap_seat]);

		sort_poker(&play_card[tidi_seat]);
		sort_poker(&play_card[swap_seat]);

		sub_poker( &all,&play_card[tidi_seat], &tmp);  //lydebug
		search_combos_in_pokers_with_laizi(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat],g->hun);

		sub_poker( &all,&play_card[swap_seat], &tmp);
		search_combos_in_pokers_with_laizi(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat],g->hun);

		if(CheckTidiness(s) <= tidiness_num) {
			c_tidi = true;
			PRINTF(LEVEL,"===CheckTidiness succeed1,switch card at %d times===\n",(16-retry));

			for (int i=0; i<3; i++)
				print_summary(&g->smry[i]);
			PRINTF(LEVEL,"\n");
			break;
		}
		retry--;

	}


	//PRINTF(LEVEL,"\n*****settidiness step2*********\n");
	clear_poker(&swap_p1);
	clear_poker(&swap_p2);
	clear_poker(&swap_all);
	clear_poker(&p1_all);
	int s_2_c = -1;
	int s_2_c1=-1;
	int s_2_c2=-1;
	POKERS temp;

	if(!c_tidi)
	{
		swap_all = play_card[swap_seat];  //g->other_pokers[swap_seat];
		p1_all = play_card[tidi_seat];
		swap_num = GetSwapArray_laizi(&p1_all,&swap_p1,s,&swap_p2,s2,&swap_all);
		clear_poker(&temp);
		add_poker(&temp,&swap_p1, &temp);
		for(int j=P3; j<Pa; j++)
		{
			clear_poker(&swap_p1);
			add_poker(&swap_p1, &temp,&swap_p1);
			if((swap_p1.hands[j]==2)&&( swap_p2.hands[j] >0))
			{

				while(1)
				{
					s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
					if(s_2_c !=j)
						break;
				}
				//PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d,single=%d\n",j+3,s_2_c+3);
				if(s_2_c >=0)
				{
					play_card[tidi_seat].hands[s_2_c]--;
					play_card[tidi_seat].hands[j]++;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[s_2_c]++;


					sort_poker( &play_card[tidi_seat]);
					sort_poker( &play_card[swap_seat]);

					//play_card[tidi_seat] = g->other_pokers[tidi_seat];
					sub_poker( &all,&play_card[tidi_seat], &tmp);
					search_combos_in_pokers_with_laizi(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat],g->hun);


					//play_card[swap_seat] = g->other_pokers[swap_seat];
					sub_poker( &all,&play_card[swap_seat], &tmp);
					search_combos_in_pokers_with_laizi(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat],g->hun);

					if(CheckTidiness(s) <= tidiness_num)
					{
						c_tidi = true;
						PRINTF(LEVEL,"===Succeed2===\n",retry);
						for (int i=0; i<3; i++)
							print_summary(&g->smry[i]);
					}
					else
					{
						clear_poker(&swap_p1);
						clear_poker(&swap_p2);
						clear_poker(&swap_all);
						clear_poker(&p1_all);
						swap_all = play_card[swap_seat];
						p1_all =  play_card[tidi_seat];
						swap_num = GetSwapArray_laizi(&p1_all,&swap_p1,s,&swap_p2,s2,&swap_all);
						clear_poker(&temp);
						add_poker(&temp,&swap_p1, &temp);
					}
				}
			}
			if(c_tidi)
				break;
		}
	}

	clear_poker(&swap_p1);
	clear_poker(&swap_p2);
	clear_poker(&swap_all);
	clear_poker(&p1_all);
	//PRINTF(LEVEL,"\n*****settidiness step3*********\n");
	if(!c_tidi)
	{
		swap_all = play_card[swap_seat];
		p1_all =  play_card[tidi_seat];
		swap_num = GetSwapArray_laizi(&p1_all,&swap_p1,s,&swap_p2,s2,&swap_all);
		clear_poker(&temp);
		add_poker(&temp,&swap_p1, &temp);

		for(int j=P3; j<Pa; j++)
		{
			clear_poker(&swap_p1);
			add_poker(&swap_p1, &temp,&swap_p1);

			if((swap_p2.hands[j]==2)&&( swap_p1.hands[j] >0))
			{
				while(1)
				{
					s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
					if(s_2_c !=j)
						break;
				}

				while(1)
				{
					s_2_c1=rand_one_exchange_poker(&swap_p1,Pk);
					if(s_2_c1 !=j)
						break;
				}
				if((s_2_c>=0)&&(s_2_c1>=0))
				{
					//PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d,single1=%d,single2=%d\n",j+3,s_2_c+3,s_2_c1+3);
					play_card[tidi_seat].hands[s_2_c]--;
					play_card[tidi_seat].hands[s_2_c1]--;
					play_card[tidi_seat].hands[j]++;
					play_card[tidi_seat].hands[j]++;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[s_2_c]++;
					play_card[swap_seat].hands[s_2_c1]++;

					sort_poker( &play_card[tidi_seat]);
					sort_poker( &play_card[swap_seat]);

					sub_poker( &all,&play_card[tidi_seat], &tmp);
					search_combos_in_pokers_with_laizi(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat],g->hun);

					sub_poker( &all,&play_card[swap_seat], &tmp);
					search_combos_in_pokers_with_laizi(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat],g->hun);

					if(CheckTidiness(s) <= tidiness_num)
					{
						c_tidi = true;
						PRINTF(LEVEL,"===Succeed2===\n",retry);
						for (int i=0; i<3; i++)
							print_summary(&g->smry[i]);
					}
					else
					{
						clear_poker(&swap_p1);
						clear_poker(&swap_p2);
						clear_poker(&swap_all);
						clear_poker(&p1_all);
						swap_all = play_card[swap_seat];
						p1_all =  play_card[tidi_seat];
						swap_num = GetSwapArray_laizi(&p1_all,&swap_p1,s,&swap_p2,s2,&swap_all);
						clear_poker(&temp);
						add_poker(&temp,&swap_p1, &temp);
					}
				}
			}
			if(c_tidi)
				break;
		}

	}
	//Find the pairs and change it to three,setp2
	clear_poker(&swap_p1);
	clear_poker(&swap_all);
	//PRINTF(LEVEL,"\n*****settidiness step4*********\n");
	if(!c_tidi)
	{
		swap_all = play_card[swap_seat];
		swap_num =GetSwapPoker_laizi(&swap_all,&swap_p1,  s,Pk);
		clear_poker(&temp);
		add_poker(&temp,&swap_p1, &temp);
		if(swap_num>=3)
		{
			for(int j=P3; j<Pa; j++)
			{
				clear_poker(&swap_p1);
				add_poker(&swap_p1, &temp,&swap_p1);
				if((swap_all.hands[j]==3))
				{
					if( swap_p1.hands[j] ==0)
					{
						s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
						if(s_2_c==-1)
							break;
						s_2_c1=rand_one_exchange_poker(&swap_p1,Pk);
						if(s_2_c1==-1)
							break;
						s_2_c2=rand_one_exchange_poker(&swap_p1,Pk);
						if(s_2_c2==-1)
							break;

						//PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d,single=%d\n",j+3,s_2_c+3);

						play_card[tidi_seat].hands[s_2_c2]--;
						play_card[tidi_seat].hands[j]++;

						play_card[swap_seat].hands[j]--;
						play_card[swap_seat].hands[s_2_c2]++;
					}
					else
					{
						while(1)
						{
							s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
							if(s_2_c !=j)
								break;
						}

						while(1)
						{
							s_2_c1=rand_one_exchange_poker(&swap_p1,Pk);
							if(s_2_c1 !=j)
								break;
						}
						// PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d\n",j+3);
					}
					play_card[tidi_seat].hands[s_2_c]--;
					play_card[tidi_seat].hands[s_2_c1]--;
					play_card[tidi_seat].hands[j]++;
					play_card[tidi_seat].hands[j]++;

					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[s_2_c]++;
					play_card[swap_seat].hands[s_2_c1]++;

					sort_poker( &play_card[tidi_seat]);
					sort_poker( &play_card[swap_seat]);

					sub_poker( &all,&play_card[tidi_seat], &tmp);
					search_combos_in_pokers_with_laizi(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat],g->hun);
					sub_poker( &all,&play_card[swap_seat], &tmp);
					search_combos_in_pokers_with_laizi(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat],g->hun);

					if(CheckTidiness(s) <= tidiness_num)
					{
						c_tidi = true;
						PRINTF(LEVEL,"===Succeed3===\n",retry);
						for (int i=0; i<3; i++)
							print_summary(&g->smry[i]);
					}
					else
					{
						clear_poker(&swap_p1);
						clear_poker(&swap_all);
						swap_all = play_card[swap_seat];
						swap_num =GetSwapPoker_laizi(&swap_all,&swap_p1,  s,Pk);
						clear_poker(&temp);
						add_poker(&temp,&swap_p1, &temp);
					}
				}
				if(c_tidi)
					break;
			}
		}
	}

	//ly09
	if(!c_tidi_0)
	{
		if( g->smry[tidi_seat].bomb_num>0)
		{
			int t=5;
			while(t)
			{
				t--;
				PRINTF(LEVEL,"===newbomb at tidiseat===\n");
				for(int i=0; i<g->smry[tidi_seat].bomb_num; i++)
				{
					DBG(print_one_poker(g->smry[tidi_seat].bomb[i]->low));

					int val=g->smry[tidi_seat].bomb[i]->low;
					int val1=rand_one_exchange_poker(&play_card[swap_seat],BIG_JOKER);
					if(val1==-1)
						break;
					play_card[tidi_seat].hands[val]--;
					play_card[tidi_seat].hands[val1]++;
					play_card[swap_seat].hands[val]++;
					//play_card[swap_seat].hands[val1]--;   //this value is subbed at rand_one_exchange_poker
				}
				sort_poker( &play_card[tidi_seat]);
				sort_poker( &play_card[swap_seat]);

				sub_poker( &all,&play_card[tidi_seat], &tmp);
				search_combos_in_pokers_with_laizi(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat],g->hun);

				if( g->smry[tidi_seat].bomb_num==0)
				{
					t=0;
					break;
				}


			}

			sub_poker( &all,&play_card[swap_seat], &tmp);
			search_combos_in_pokers_with_laizi(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat],g->hun);

			if(CheckTidiness(s) <= tidiness_num)
			{
				c_tidi = true;
				PRINTF(LEVEL,"===Succeed4===\n",retry);
				for (int i=0; i<3; i++)
					print_summary(&g->smry[i]);
			}
			else
				c_tidi = false;

		}

	}

	for(int i=0;i<3;i++)
		add_poker(&play_card[i], &sbomb[i], &play_card[i]);

	//update other pokers
	g->other_pokers[tidi_seat] =play_card[tidi_seat];
	g->other_pokers[swap_seat] =play_card[swap_seat];

	//check the pot
	if(tidi_seat==first_seat)
	{
		if(is_sub_poker(&g->other_pokers[3], &g->other_pokers[first_seat]))
			sub_poker(&g->other_pokers[first_seat],&g->other_pokers[3], &g->other_pokers[first_seat]);

		else {
			//rand 3 cards to pot
			clear_poker(&g->other_pokers[3]);
			for (int k=0; k<3; k++)
			{
				int val=get_pos_char(g->other_pokers[tidi_seat].hands,P2);
				if(val==-1)
					val=get_pos_char(g->other_pokers[tidi_seat].hands,MAX_POKER_KIND);
				g->other_pokers[3].hands[val]++;
				g->other_pokers[tidi_seat].hands[val]--;
			}
			sort_poker(&g->other_pokers[first_seat]);  //ly07
			sort_poker(&g->other_pokers[3]);
		}   //ly07
	}

	if(!c_tidi ){
		PRINTF(LEVEL,"\n*****Settidiness is failed*****\n");
	}

	PRINTF(LEVEL,"\n*****Settidiness is finished*****\n");

}


void  settidiness(lordGame* g, int tidi_seat, int swap_seat, int first_seat)
{
	int tidiness_num=2;   //ly20140325 add

	POKERS play_card[3],tmp,all;
	//full_poker(&all);
	clear_poker(&all);
	for (int i=0; i<4; i++)
		add_poker( &all,&g->other_pokers[i], &all);

	for (int i=0; i<3; i++)
	{
		play_card[i] = g->other_pokers[i];
		if(i==first_seat)
		{
			add_poker( &g->other_pokers[3],&play_card[i], &play_card[i]);
		}
		sub_poker( &all,&play_card[i], &tmp);
		search_combos_in_pokers(&play_card[i], &tmp,
			g->cs[i], &g->smry[i]);
		print_summary(&g->smry[i]);
	}

	COMBOS_SUMMARY *s = &g->smry[tidi_seat];
	COMBOS_SUMMARY *s2 = &g->smry[swap_seat];

	//ly20140325 add start

	PRINTF(LEVEL,"===settidiness tidiseat is %d,swap seat is %d,first seat is %d ===\n",tidi_seat,swap_seat,first_seat);

	POKERS swap_p1,swap_p2,swap_all;
	int swap_num;
	int retry=15;
	bool c_tidi= false;
	bool c_tidi_0 =false;
	if(CheckTidiness(s) <= tidiness_num) {
		c_tidi = true;
		c_tidi_0 = true;
		PRINTF(LEVEL,"===CheckTidiness succeed0===\n");
	}

	while(retry)
	{
		clear_poker(&swap_p1);
		clear_poker(&swap_p2);
		clear_poker(&swap_all);


		//ly06
		//Put all the other pokers to swap_all
		swap_all = play_card[swap_seat];   //g->other_pokers[swap_seat];
		swap_num = GetSwapArray(&swap_p1,s,&swap_p2,s2,&swap_all);

		//Begin to exchange num_p1 pokers in P1( swap_p1) and P2(swap_p2)

		sub_poker(&play_card[tidi_seat],&swap_p1,&play_card[tidi_seat]);
		sub_poker(&play_card[swap_seat],&swap_p2,&play_card[swap_seat]);


		add_poker(&play_card[tidi_seat],&swap_p2,&play_card[tidi_seat]);
		add_poker(&play_card[swap_seat],&swap_p1,&play_card[swap_seat]);

		sort_poker(&play_card[tidi_seat]);
		sort_poker(&play_card[swap_seat]);

		sub_poker( &all,&play_card[tidi_seat], &tmp);
		search_combos_in_pokers(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat]);

		sub_poker( &all,&play_card[swap_seat], &tmp);
		search_combos_in_pokers(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat]);

		if(CheckTidiness(s) <= tidiness_num) {
			c_tidi = true;

			PRINTF(LEVEL,"===CheckTidiness succeed1,switch card at %d times===\n",(16-retry));

			for (int i=0; i<3; i++)
				print_summary(&g->smry[i]);
			PRINTF(LEVEL,"\n");
			break;
		}
		retry--;

	}


	//PRINTF(LEVEL,"\n*****settidiness step2*********\n");
	clear_poker(&swap_p1);
	clear_poker(&swap_p2);
	clear_poker(&swap_all);

	int s_2_c = -1;
	int s_2_c1=-1;
	int s_2_c2=-1;
	POKERS temp;

	if(!c_tidi)
	{
		swap_all = play_card[swap_seat];  //g->other_pokers[swap_seat];
		swap_num = GetSwapArray(&swap_p1,s,&swap_p2,s2,&swap_all);
		clear_poker(&temp);
		add_poker(&temp,&swap_p1, &temp);
		for(int j=P3; j<Pa; j++)
		{
			clear_poker(&swap_p1);
			add_poker(&swap_p1, &temp,&swap_p1);
			if((swap_p1.hands[j]==2)&&( swap_p2.hands[j] >0))
			{

				while(1)
				{
					s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
					if(s_2_c !=j)
						break;
				}
				//PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d,single=%d\n",j+3,s_2_c+3);
				if(s_2_c >=0)
				{
					play_card[tidi_seat].hands[s_2_c]--;
					play_card[tidi_seat].hands[j]++;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[s_2_c]++;


					sort_poker( &play_card[tidi_seat]);
					sort_poker( &play_card[swap_seat]);

					//play_card[tidi_seat] = g->other_pokers[tidi_seat];
					sub_poker( &all,&play_card[tidi_seat], &tmp);
					search_combos_in_pokers(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat]);


					//play_card[swap_seat] = g->other_pokers[swap_seat];
					sub_poker( &all,&play_card[swap_seat], &tmp);
					search_combos_in_pokers(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat]);

					if(CheckTidiness(s) <= tidiness_num)
					{
						c_tidi = true;
						PRINTF(LEVEL,"===Succeed2===\n",retry);
						for (int i=0; i<3; i++)
							print_summary(&g->smry[i]);
					}
					else
					{
						clear_poker(&swap_p1);
						clear_poker(&swap_p2);
						clear_poker(&swap_all);
						swap_all = play_card[swap_seat];
						swap_num = GetSwapArray(&swap_p1,s,&swap_p2,s2,&swap_all);
						clear_poker(&temp);
						add_poker(&temp,&swap_p1, &temp);
					}
				}
			}
			if(c_tidi)
				break;
		}
	}

	clear_poker(&swap_p1);
	clear_poker(&swap_p2);
	clear_poker(&swap_all);
	//PRINTF(LEVEL,"\n*****settidiness step3*********\n");
	if(!c_tidi)
	{
		swap_all = play_card[swap_seat];
		swap_num = GetSwapArray(&swap_p1,s,&swap_p2,s2,&swap_all);
		clear_poker(&temp);
		add_poker(&temp,&swap_p1, &temp);

		for(int j=P3; j<Pa; j++)
		{
			clear_poker(&swap_p1);
			add_poker(&swap_p1, &temp,&swap_p1);

			if((swap_p2.hands[j]==2)&&( swap_p1.hands[j] >0))
			{
				while(1)
				{
					s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
					if(s_2_c !=j)
						break;
				}

				while(1)
				{
					s_2_c1=rand_one_exchange_poker(&swap_p1,Pk);
					if(s_2_c1 !=j)
						break;
				}
				if((s_2_c>=0)&&(s_2_c1>=0))
				{
					//PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d,single1=%d,single2=%d\n",j+3,s_2_c+3,s_2_c1+3);
					play_card[tidi_seat].hands[s_2_c]--;
					play_card[tidi_seat].hands[s_2_c1]--;
					play_card[tidi_seat].hands[j]++;
					play_card[tidi_seat].hands[j]++;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[s_2_c]++;
					play_card[swap_seat].hands[s_2_c1]++;

					sort_poker( &play_card[tidi_seat]);
					sort_poker( &play_card[swap_seat]);

					sub_poker( &all,&play_card[tidi_seat], &tmp);
					search_combos_in_pokers(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat]);

					sub_poker( &all,&play_card[swap_seat], &tmp);
					search_combos_in_pokers(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat]);

					if(CheckTidiness(s) <= tidiness_num)
					{
						c_tidi = true;
						PRINTF(LEVEL,"===Succeed2===\n",retry);
						for (int i=0; i<3; i++)
							print_summary(&g->smry[i]);
					}
					else
					{
						clear_poker(&swap_p1);
						clear_poker(&swap_p2);
						clear_poker(&swap_all);
						swap_all = play_card[swap_seat];
						swap_num = GetSwapArray(&swap_p1,s,&swap_p2,s2,&swap_all);
						clear_poker(&temp);
						add_poker(&temp,&swap_p1, &temp);
					}
				}
			}
			if(c_tidi)
				break;
		}

	}
	//Find the pairs and change it to three,setp2
	clear_poker(&swap_p1);
	clear_poker(&swap_all);
	//PRINTF(LEVEL,"\n*****settidiness step4*********\n");
	if(!c_tidi)
	{
		swap_all = play_card[swap_seat];
		swap_num =GetSwapPoker(&swap_p1,  s,Pk);
		clear_poker(&temp);
		add_poker(&temp,&swap_p1, &temp);
		if(swap_num>=3)
		{
			for(int j=P3; j<Pa; j++)
			{
				clear_poker(&swap_p1);
				add_poker(&swap_p1, &temp,&swap_p1);
				if((swap_all.hands[j]==3))
				{
					if( swap_p1.hands[j] ==0)
					{
						s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
						if(s_2_c==-1)
							break;
						s_2_c1=rand_one_exchange_poker(&swap_p1,Pk);
						if(s_2_c1==-1)
							break;
						s_2_c2=rand_one_exchange_poker(&swap_p1,Pk);
						if(s_2_c2==-1)
							break;

						//PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d,single=%d\n",j+3,s_2_c+3);

						play_card[tidi_seat].hands[s_2_c2]--;
						play_card[tidi_seat].hands[j]++;

						play_card[swap_seat].hands[j]--;
						play_card[swap_seat].hands[s_2_c2]++;
					}
					else
					{
						while(1)
						{
							s_2_c=rand_one_exchange_poker(&swap_p1,Pk);
							if(s_2_c !=j)
								break;
						}

						while(1)
						{
							s_2_c1=rand_one_exchange_poker(&swap_p1,Pk);
							if(s_2_c1 !=j)
								break;
						}
						// PRINTF(LEVEL,"\n*****find a single for pairs*****pairs=%d\n",j+3);
					}
					play_card[tidi_seat].hands[s_2_c]--;
					play_card[tidi_seat].hands[s_2_c1]--;
					play_card[tidi_seat].hands[j]++;
					play_card[tidi_seat].hands[j]++;

					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[j]--;
					play_card[swap_seat].hands[s_2_c]++;
					play_card[swap_seat].hands[s_2_c1]++;

					sort_poker( &play_card[tidi_seat]);
					sort_poker( &play_card[swap_seat]);

					sub_poker( &all,&play_card[tidi_seat], &tmp);
					search_combos_in_pokers(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat]);
					sub_poker( &all,&play_card[swap_seat], &tmp);
					search_combos_in_pokers(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat]);

					if(CheckTidiness(s) <= tidiness_num)
					{
						c_tidi = true;
						PRINTF(LEVEL,"===Succeed3===\n",retry);
						for (int i=0; i<3; i++)
							print_summary(&g->smry[i]);
					}
					else
					{
						clear_poker(&swap_p1);
						clear_poker(&swap_all);
						swap_all = play_card[swap_seat];
						swap_num =GetSwapPoker(&swap_p1,  s,Pk);
						clear_poker(&temp);
						add_poker(&temp,&swap_p1, &temp);
					}
				}
				if(c_tidi)
					break;
			}
		}
	}

	//ly09
	if(!c_tidi_0)
	{
		if( g->smry[tidi_seat].bomb_num>0)
		{
			int t=5;
			while(t)
			{
				t--;
				PRINTF(LEVEL,"===newbomb at tidiseat===\n");
				for(int i=0; i<g->smry[tidi_seat].bomb_num; i++)
				{
					DBG(print_one_poker(g->smry[tidi_seat].bomb[i]->low));

					int val=g->smry[tidi_seat].bomb[i]->low;
					int val1=rand_one_exchange_poker(&play_card[swap_seat],BIG_JOKER);
					if(val1==-1)
						break;
					play_card[tidi_seat].hands[val]--;
					play_card[tidi_seat].hands[val1]++;
					play_card[swap_seat].hands[val]++;
					//play_card[swap_seat].hands[val1]--;   //this value is subbed at rand_one_exchange_poker
				}
				sort_poker( &play_card[tidi_seat]);
				sort_poker( &play_card[swap_seat]);

				sub_poker( &all,&play_card[tidi_seat], &tmp);
				search_combos_in_pokers(&play_card[tidi_seat], &tmp,g->cs[tidi_seat], &g->smry[tidi_seat]);

				if( g->smry[tidi_seat].bomb_num==0)
				{
					t=0;
					break;
				}


			}

			sub_poker( &all,&play_card[swap_seat], &tmp);
			search_combos_in_pokers(&play_card[swap_seat], &tmp,g->cs[swap_seat], &g->smry[swap_seat]);

			if(CheckTidiness(s) <= tidiness_num)
			{
				c_tidi = true;
				PRINTF(LEVEL,"===Succeed4===\n",retry);
				for (int i=0; i<3; i++)
					print_summary(&g->smry[i]);
			}
			else
				c_tidi = false;

		}

	}



	//update other pokers
	g->other_pokers[tidi_seat] =play_card[tidi_seat];
	g->other_pokers[swap_seat] =play_card[swap_seat];

	//check the pot
	if(tidi_seat==first_seat)
	{
		if(is_sub_poker(&g->other_pokers[3], &g->other_pokers[first_seat]))
			sub_poker(&g->other_pokers[first_seat],&g->other_pokers[3], &g->other_pokers[first_seat]);

		else {
			//rand 3 cards to pot
			clear_poker(&g->other_pokers[3]);
			for (int k=0; k<3; k++)
			{
				int val=get_pos_char(g->other_pokers[tidi_seat].hands,P2);
				if(val==-1)
					val=get_pos_char(g->other_pokers[tidi_seat].hands,MAX_POKER_KIND);
				g->other_pokers[3].hands[val]++;
				g->other_pokers[tidi_seat].hands[val]--;
			}
			sort_poker(&g->other_pokers[first_seat]);  //ly07
			sort_poker(&g->other_pokers[3]);
		}   //ly07
	}

	if(!c_tidi ){
		PRINTF(LEVEL,"\n*****Settidiness is failed*****\n");
	}
}

//cyl 20160531 二人斗地主
//调整牌的整齐度
void  settidiness_two(lordGame* g, int tidi_seat, int swap_seat, int first_seat)
{
	//先去除3 4
	POKERS * pokers = &(g->other_pokers[2]);
	pokers->hands[P3] = 0;
	pokers->hands[P4] = 0;
	sort_poker(pokers);

	//优化牌
	settidiness(g, tidi_seat, swap_seat, first_seat);

	//再先加上3 4
	pokers->hands[P3] = 4;
	pokers->hands[P4] = 4;
	sort_poker(pokers);
}
//调整最优座
void  setBetterSeat_two(lordGame* g, int pos)
{
	int num = 2;
	if (pos < 0 || pos > num)
	{
		return;
	}
	int cur_pos = -1;
	//Step1 Judge the number of the bomb
	for (int i = 0; i < num; i++)
	{
		if (g->bomb_number[i] > g->bomb_number[(i + 1) % num])
		{
			cur_pos = i;
		}
	}
	//Step2 Judge the number of the king+2
	if (cur_pos == -1)
	{
		int num = 0;
		int num_wang_2[2] = { 0 };
		for (int i = 0; i < num; i++)
		{
			num_wang_2[i] = calc_wang_2(&g->other_pokers[i]);
		}
		for (int i = 0; i < num; i++)
		{
			if (num_wang_2[i] > num_wang_2[(i + 1) % num])
			{
				cur_pos = i;
			}
		}
	}
	//Setp3 Judge the Dawang
	if (cur_pos == -1)
	{
		for (int i = 0; i < num; i++)
		{
			if (g->other_pokers[i].hands[BIG_JOKER])
			{
				cur_pos = i;
			}
		}
	}
	//Setp4 try swap
	if (cur_pos != -1 && cur_pos != pos)
	{
		swap_card(g, pos, cur_pos);
	}
}

//ly20140325 add end

void  setBetterSeat_laizi(lordGame* g, int pos)
{

	POKERS pall;
	clear_poker(&pall);

	if(pos==3)
		return;

	int cur_pos=-1, skip_pos=-1;
	if( g->bomb_number[2]>g->bomb_number[1])
	{
		if(g->bomb_number[2]>g->bomb_number[0])
		{
			cur_pos=2;
		}
		else if(g->bomb_number[2]<g->bomb_number[0])
		{
			cur_pos =0;
		}
		else // if(g->bomb_number[2]==g->bomb_number[0])
		{
			skip_pos=1;
		}

	}
	else   if( g->bomb_number[2]<g->bomb_number[1])
	{
		if(g->bomb_number[1] > g->bomb_number[0])
		{
			cur_pos= 1;
		}
		else if(g->bomb_number[1]<g->bomb_number[0])
		{
			cur_pos =0;
		}
		else // if(g->bomb_number[1]==g->bomb_number[0])
		{
			skip_pos=2;
		}
	}
	else  // if(g->bomb_number[1]==g->bomb_number[2])
	{
		if(g->bomb_number[1]<g->bomb_number[0])
		{
			cur_pos =0;
		}
		//ly20140314 add begin
		else if(g->bomb_number[1]>g->bomb_number[0])
		{
			skip_pos=0;
		}
	}
	//ly20140314 add end

	//ly20140314 add begin
	//Step2 Judge the number of the king+2
	if(cur_pos==-1)
	{
		//int num=0;
		int num_wang_2[3]= {0,0,0};
		for (int i=0; i<3; i++)
		{
			if(i==skip_pos)
				continue;
			num_wang_2[i]= calc_wang_2(&g->other_pokers[i]);
		}
		switch(skip_pos)
		{
		case -1:
			if( num_wang_2[2]>num_wang_2[1])
			{
				if(num_wang_2[2]>num_wang_2[0])
					cur_pos=2;
				else if(num_wang_2[2]<num_wang_2[0])
					cur_pos =0;
				else
					skip_pos=1;
			}
			else if( num_wang_2[2]<num_wang_2[1])
			{
				if(num_wang_2[1] > num_wang_2[0])
					cur_pos= 1;
				else if(num_wang_2[1]<num_wang_2[0])
					cur_pos =0;
				else
					skip_pos=2;
			}
			else
			{
				if(num_wang_2[1]<num_wang_2[0])
					cur_pos =0;
				else if(num_wang_2[1]>num_wang_2[0])
					skip_pos=0;
			}

			break;

		case 0:
			if(num_wang_2[1]>num_wang_2[2])
				cur_pos = 1;
			else if (num_wang_2[1]<num_wang_2[2])
				cur_pos =2;
			break;

		case 1:
			if(num_wang_2[0]>num_wang_2[2])
				cur_pos = 0;
			else if (num_wang_2[0]<num_wang_2[2])
				cur_pos =2;
			break;

		case 2:
			if(num_wang_2[0]>num_wang_2[1])
				cur_pos = 0;
			else if (num_wang_2[0]<num_wang_2[1])
				cur_pos =1;
			break;
		}
	}

	//Setp3 Judge the Dawang
	if(cur_pos==-1)
	{
		for (int i=0; i<3; i++)
		{
			if(i==skip_pos)
				continue;
			if(g->other_pokers[i].hands[BIG_JOKER])
			{
				cur_pos = i;
			}
		}
	}
	//Setp4 Select the smallest seat
	if(cur_pos==-1)
	{
		if(skip_pos == 0)
			cur_pos = 1;
		else
			cur_pos = 0;
	}
	//PRINTF(LEVEL,"===the best seat %d is found ===\n",cur_pos);
	//ly20140314 add end

	if( cur_pos != -1) //found a better seat
	{
		//swap cur_pos with pos
		if(cur_pos!=pos)
		{
			swap_card(g,pos,cur_pos);
		}
	}

	if(g->hun == -1)
	{
		add_poker( &pall,&g->other_pokers[pos], &pall);
		add_poker( &pall,&g->bombs[pos], &pall);
		g->hun =  GetHun(&pall);
		PRINTF(LEVEL,"===GetHun, hun=%d ===\n",g->hun);
	}

	return;
}


void  setBetterSeat(lordGame* g, int pos)
{
	if(pos==3)
		return;

	int cur_pos=-1, skip_pos=-1;
	if( g->bomb_number[2]>g->bomb_number[1])
	{
		if(g->bomb_number[2]>g->bomb_number[0])
		{
			cur_pos=2;
		}
		else if(g->bomb_number[2]<g->bomb_number[0])
		{
			cur_pos =0;
		}
		else // if(g->bomb_number[2]==g->bomb_number[0])
		{
			skip_pos=1;
		}

	}
	else   if( g->bomb_number[2]<g->bomb_number[1])
	{
		if(g->bomb_number[1] > g->bomb_number[0])
		{
			cur_pos= 1;
		}
		else if(g->bomb_number[1]<g->bomb_number[0])
		{
			cur_pos =0;
		}
		else // if(g->bomb_number[1]==g->bomb_number[0])
		{
			skip_pos=2;
		}
	}
	else  // if(g->bomb_number[1]==g->bomb_number[2])
	{
		if(g->bomb_number[1]<g->bomb_number[0])
		{
			cur_pos =0;
		}
		//ly20140314 add begin
		else if(g->bomb_number[1]>g->bomb_number[0])
		{
			skip_pos=0;
		}
	}
	//ly20140314 add end

	//ly20140314 add begin
	//Step2 Judge the number of the king+2
	if(cur_pos==-1)
	{
		//int num=0;
		int num_wang_2[3]= {0,0,0};
		for (int i=0; i<3; i++)
		{
			if(i==skip_pos)
				continue;
			num_wang_2[i]= calc_wang_2(&g->other_pokers[i]);
		}
		switch(skip_pos)
		{
		case -1:
			if( num_wang_2[2]>num_wang_2[1])
			{
				if(num_wang_2[2]>num_wang_2[0])
					cur_pos=2;
				else if(num_wang_2[2]<num_wang_2[0])
					cur_pos =0;
				else
					skip_pos=1;
			}
			else if( num_wang_2[2]<num_wang_2[1])
			{
				if(num_wang_2[1] > num_wang_2[0])
					cur_pos= 1;
				else if(num_wang_2[1]<num_wang_2[0])
					cur_pos =0;
				else
					skip_pos=2;
			}
			else
			{
				if(num_wang_2[1]<num_wang_2[0])
					cur_pos =0;
				else if(num_wang_2[1]>num_wang_2[0])
					skip_pos=0;
			}

			break;

		case 0:
			if(num_wang_2[1]>num_wang_2[2])
				cur_pos = 1;
			else if (num_wang_2[1]<num_wang_2[2])
				cur_pos =2;
			break;

		case 1:
			if(num_wang_2[0]>num_wang_2[2])
				cur_pos = 0;
			else if (num_wang_2[0]<num_wang_2[2])
				cur_pos =2;
			break;

		case 2:
			if(num_wang_2[0]>num_wang_2[1])
				cur_pos = 0;
			else if (num_wang_2[0]<num_wang_2[1])
				cur_pos =1;
			break;
		}
	}

	//Setp3 Judge the Dawang
	if(cur_pos==-1)
	{
		for (int i=0; i<3; i++)
		{
			if(i==skip_pos)
				continue;
			if(g->other_pokers[i].hands[BIG_JOKER])
			{
				cur_pos = i;
			}
		}
	}
	//Setp4 Select the smallest seat
	if(cur_pos==-1)
	{
		if(skip_pos == 0)
			cur_pos = 1;
		else
			cur_pos = 0;
	}
	//PRINTF(LEVEL,"===the best seat %d is found ===\n",cur_pos);
	//ly20140314 add end

	if( cur_pos != -1) //found a better seat
	{
		//swap cur_pos with pos
		if(cur_pos!=pos)
		{
			swap_card(g,pos,cur_pos);
		}
	}
	return;
}


void setGoodPot(lordGame* g, int pos)
{
	//change its card?
	POKERS tmp;
	add_poker(&g->other_pokers[pos],
		&g->other_pokers[3],
		&tmp);

	add_poker(&g->bombs[pos],
		&tmp,
		&tmp);    //tmp include all pokers of firstseat

	POKERS all,opp,newPot;
	clear_poker(&newPot);
	full_poker(&all);
	sub_poker(&all,&tmp,&opp);
	POKERS tpot;
	clear_poker(&tpot);

	COMBOS_SUMMARY *s=&g->smry[pos];

	//search_combos_in_pokers_with_laizi(&tmp,&opp,g->cs[pos],s,g->hun);
	search_combos_in_pokers(&tmp,&opp,g->cs[pos],s);
#if 0
	PRINTF(LEVEL,"\n===setGoodPot===\n");
	print_hand_poker_in_line(&tmp);
	PRINTF(LEVEL,"\n");
	print_summary(s);
#endif
	int num_pos = 0;

	// 1.Select N control
	//Add bomb
	if(s->bomb_num >=1)
	{
		for (int k=0; k<s->bomb_num; k++)
		{
			int val=s->bomb[k]->low;
			if(val==LIT_JOKER)
			{
				INSERT_A_CARD(&tpot,LIT_JOKER);
				INSERT_A_CARD(&tpot,BIG_JOKER);
				//  num_pos++;
			}
			else
			{
				INSERT_A_CARD(&tpot,s->bomb[k]->low);
				INSERT_A_CARD(&tpot,s->bomb[k]->low);
				//   INSERT_A_CARD(&tpot,s->bomb[k]->low);
				//  INSERT_A_CARD(&tpot,s->bomb[k]->low);
				//  num_pos++;
			}
		}


	}
	//Add Wang+2
	for (int k=0; k<s->singles_num; k++)
	{
		if(( s->singles[k]->low>=P2)&&( s->singles[k]->low<MAX_POKER_KIND))
		{
			INSERT_A_CARD(&tpot, s->singles[k]->low);
			num_pos++;
		}
	}

	num_pos+= s->bomb_num;
	if(num_pos>1)
		num_pos--;
	for(int i =0; i<num_pos; i++)
	{
		int val=rand_a_poker(&tpot);
		INSERT_A_CARD(&newPot,val);
	}

	//Add laizi
	if(g->hun!=-1)
	{
		for(int k=0;k<tmp.hands[g->hun];k++)
		{
			INSERT_A_CARD(&newPot,g->hun);
			num_pos++;
		}
	}
#if 0
	if(num_pos>0) {
		PRINTF(LEVEL,"\n===SetGoodPot Step1=pos number is%d===\n",num_pos);
		print_hand_poker_in_line(&newPot);
	}
#endif
	// 2.Select	threeseries_one and threeseries_two

	for (int k=0; k< s->threeseries_one_num; k++)
	{
		int val= RAND_NUM_FROM_RANGE(s->threeseries_one[k]->low,s->threeseries_one[k]->low+1);
		INSERT_A_CARD(&newPot,val);
		val= RAND_NUM_FROM_RANGE(s->threeseries_one[k]->low,s->threeseries_one[k]->low+1);
		INSERT_A_CARD(&newPot,val);
		num_pos+=2;
	}


	for (int k=0; k< s->threeseries_two_num; k++)
	{
		int val= RAND_NUM_FROM_RANGE(s->threeseries_two[k]->low,s->threeseries_two[k]->low+1);
		INSERT_A_CARD(&newPot,val);
		val= RAND_NUM_FROM_RANGE(s->threeseries_two[k]->low,s->threeseries_two[k]->low+1);
		INSERT_A_CARD(&newPot,val);
		num_pos+=2;
	}
	if(num_pos>0) {
		PRINTF(LEVEL,"\n===SetGoodPot Step2 pos number is =%d===\n",num_pos);
		print_hand_poker_in_line(&newPot);
	}
	// 3.Select pairs_series

	for(int i=0; i<s->pairs_series_num; i++)
	{
		int val =RAND_NUM_FROM_RANGE(s->pairs_series[i]->low,
			s->pairs_series[i]->low+ s->pairs_series[i]->len-1);
		INSERT_A_CARD(&newPot,val);

		val= RAND_NUM_FROM_RANGE(s->pairs_series[i]->low,
			s->pairs_series[i]->low+ s->pairs_series[i]->len-1);
		INSERT_A_CARD(&newPot,val);
		num_pos+=2;

	}
	if(num_pos>0) {
		//PRINTF(LEVEL,"\n===SetGoodPot Step3 pos number is=%d===\n",num_pos);
		print_hand_poker_in_line(&newPot);
	}
	// 4. Select single series
	for (int i=0; i< s->series_num; i++)
	{
		int val= RAND_NUM_FROM_RANGE(s->series[i]->low+1,
			s->series[i]->low+s->series[i]->len-2);
		INSERT_A_CARD(&newPot,val);

		int a=val;
		while(a==val)
		{
			val= RAND_NUM_FROM_RANGE(s->series[i]->low+1,
				s->series[i]->low+s->series[i]->len-2);

		}
		INSERT_A_CARD(&newPot,val);
		num_pos+=2;
	}
	/*	
	if(num_pos>0) {
	PRINTF(LEVEL,"\n===SetGoodPot Step4 pos number is=%d===",num_pos);
	print_hand_poker_in_line(&newPot);
	PRINTF(LEVEL,"\n");		
	}	*/

	//5. Remove singles <2
	for (int k=0; k<s->singles_num; k++)
	{
		if( s->singles[k]->low<P2)
		{
			if(newPot.hands[(int)(s->singles[k]->low)])
			{
				num_pos-= newPot.hands[(int)(s->singles[k]->low)];
				newPot.hands[(int)(s->singles[k]->low)]=0;
				PRINTF(LEVEL,"\n===SetGoodPot Step5 pos %d removed,pot number is=%d===\n",k+3,num_pos);
			}
		}
	}

	for (int k=0; k<s->three_one_num; k++)
	{
		int single= s->three_one[k]->three_desc[0];
		{
			if(newPot.hands[single] && single < P2)
			{
				num_pos-= newPot.hands[single];
				newPot.hands[single]=0;
				PRINTF(LEVEL,"\n===SetGoodPot Step5 pos %d removed,pot number is=%d===\n",k+3,num_pos);
			}
		}
	}

	//6. Add three and biggest 2 paris
	if(num_pos<3)
	{
		for (int i=0; i< s->three_one_num; i++)
		{
			INSERT_A_CARD(&newPot,s->three_one[i]->low);
			INSERT_A_CARD(&newPot,s->three_one[i]->low);
			INSERT_A_CARD(&newPot,s->three_one[i]->low);
			num_pos+=3;
		}
		for (int i=0; i< s->three_two_num; i++)
		{
			INSERT_A_CARD(&newPot,s->three_two[i]->low);
			INSERT_A_CARD(&newPot,s->three_two[i]->low);
			INSERT_A_CARD(&newPot,s->three_two[i]->low);
			num_pos+=3;
		}
		if( s->pairs_num==1)
		{
			INSERT_A_CARD(&newPot,s->pairs[0]->low);
			INSERT_A_CARD(&newPot,s->pairs[0]->low);
			num_pos+=2;
		}
		else if(s->pairs_num>=2)
		{
			for(int i=s->pairs_num-2; i<s->pairs_num; i++)
			{
				INSERT_A_CARD(&newPot,s->pairs[i]->low);
				INSERT_A_CARD(&newPot,s->pairs[i]->low);
				num_pos+=2;
			}

		}
#if 0
		if(num_pos>0) {
			PRINTF(LEVEL,"\n===SetGoodPot Step6 pos number is=%d===\n",num_pos);
			print_hand_poker_in_line(&newPot);
		}
#endif
	}

	//7.Check laizi
	if(g->hun!=-1){
		for(int i=0;i<MAX_POKER_KIND;i++)
		{
			int num1=newPot.hands[i];
			int num2=tmp.hands[i];
			if(num1>num2)
			{
				newPot.hands[i]=num2;
				num_pos-=num1-num2;
			}
		}
	}
	//8.
	clear_poker(&tpot);
	if(num_pos<3)
	{

		sub_poker(&tmp, &newPot,&tpot);
		for(int i =0; i<3; i++)
		{
			int val=rand_a_poker(&tpot);
			INSERT_A_CARD(&newPot,val);
		}
	}
	//PRINTF(LEVEL,"\n===setGoodPot Get the newPot below===\n");

	//update pos
	clear_poker(&g->other_pokers[3]);

	for(int i =0; i<3; i++)
	{
		int val=rand_a_poker(&newPot);
		INSERT_A_CARD(&g->other_pokers[3],val);
	}
	sort_poker(&g->other_pokers[3]);

	//PRINTF(LEVEL,"\n===setGoodPot &g->other_pokers[3]below===\n");
	//print_hand_poker_in_line(&g->other_pokers[3]);

	POKERS temp1;
	clear_poker(&temp1);

	sub_poker(&tmp,&g->other_pokers[3],&temp1);
	sort_poker(&temp1);

	//PRINTF(LEVEL,"\n===setGoodPot &temp1 below===\n");
	//print_hand_poker_in_line(&temp1);

	clear_poker(&g->bombs[pos]);
	g->other_pokers[pos]=temp1;

}

/*cardList为54张牌，其中0~16为座位0玩家手牌，
17~33为座位1玩家手牌，34~50为座位2玩家手牌，51~53为底牌*/
//#ifndef __INIT_DEBUG
static void  convert_to_int(lordGame *g, int *card)
{
	int type[4]= {1,1,1,1};
	int* pcard[4];
	for(int j=0; j<4; j++)
		pcard[j]= card+17*j;
	for(int val=P3; val<=BIG_JOKER; val++)
	{
		for(int j=0; j<4; j++)
		{
			for(int k=0; k<g->other_pokers[j].hands[val]; k++)
			{
				int t=get_pos(type, 4);
				type[t]=0;
				if(val>P2)
					*(pcard[j])=39+ val;
				else
					*(pcard[j])=t*13 + val;
				pcard[j]++;
			}
		}
		for(int j=0; j<4; j++)
			type[j]=1;
	}
}

int get_bomb_number_for_bxp()
{
	int bomb_proc[9] = {
		0, // no bombs
		0,// 1 bombs
		0,// 2 bombs
		5,// 3 bombs
		15,// 4 bombs
		35,// 5 bombs
		25,// 6 bombs
		15,// 7 bombs
		5,// 8 bombs
	};
	int total = 0;

	for (int i = 0; i < 9; ++i) {
		bomb_proc[i] *= 10;
	}

	for (int i = 0; i < 9; ++i) {
		total += bomb_proc[i];
	}

	int rnd = rand() % total + 1;
	// cout << "rnd" << rnd << endl;
	int res = 0;
	for (int i = 0; i < 9; ++i) {
		if (bomb_proc[i] > rnd)
		{
			res = i;
			break;
		}
		rnd -= bomb_proc[i];
	}

	return res;
}

bool InitCardRandom(Request& cardReq, int inRuleType, int *Card, int* Laizi)
{
	lordGame * game=(lordGame*)malloc(sizeof(lordGame));
	lordGame * g=game;
	if(game ==NULL){
		return false;
	}

	reset_game(g);

	int bomb_number=0, multi=0;    //=cardReq.bomb_multiple;  
	switch(cardReq.bomb_multiple)
	{
	case 4:
		multi=1;
		break;
	case 16:
		multi=2;
		break;
	case 64:
		multi=4;
		break;
	case 256:
		multi=8;
		break;
	case 1024:
		multi=16;
		break;
	}
	if(inRuleType != 1){
		multi= cardReq.bomb_multiple;
	}

	if (multi > 0)
	{
		while (multi >>= 1)
		{
			bomb_number++;
		}
	}

	bomb_number = CLIP3(bomb_number,0,8);
	int first_pos = CLIP3(cardReq.call_lord_first,0,2);
	int better_pos =CLIP3(cardReq.better_seat,0,3);
	
	if (cardReq.bomb_multiple == -2)
	{
		bomb_number = get_bomb_number_for_bxp();
	}


	// step 0: bomb
	setbomb(game, bomb_number, inRuleType);

	if(!verify_game(game, inRuleType)){
		free(game);
		return 0;
	}

	if(0)  //0->1 lytest
	{
		PRINTF(LEVEL,"++after set bomb++\n");
		dumpGame(game);
	}
	game->hun = -1;
	// step 1:  better seat
	if(better_pos <3)
	{
		if (inRuleType == 2)
			setBetterSeat_two(game, better_pos);
		else if(inRuleType==0 || inRuleType == 3)
			setBetterSeat( game, better_pos);
		else
			setBetterSeat_laizi( game, better_pos);
	}        
	//ly20130326 remove   else if (cardReq->base_good) //give first player better
	//ly20130326 remove        setBetterSeat( game, first_pos);

	if(0)
	{
		PRINTF(LEVEL,"++after set better++\n");
		dumpGame(game);
	}

	// step 2:  tidiness
	int pos_better=-1, pos_first=-1;//优势位 第一个发牌
	int tidi= cardReq.card_tidiness;//是否调整手牌的整齐度
	int better_seat = better_pos;//是否有优势位 3-优势位
	int first_seat =first_pos ;//默认地主 0
	int base_good = cardReq.base_good;//是否调整默认地主的牌
	int swap[2]={0};
	//Check the seats need to be ajusted

	tidi=1;
	if((tidi==0)&&(base_good==0))  //ly20140325
	{
		PRINTF(LEVEL,"++settidiness not entered++\n");
	}
	else
	{
		if( better_seat<3)
		{
			pos_better =better_seat;
			if(base_good==1)
			{
				pos_first=first_seat;
				if(better_seat == first_seat)
					pos_better = -1;
			}
			else if(base_good==0)
			{
				if(better_seat == first_seat)
				{
					pos_better = -1;
					pos_first= first_seat;
				}
			}
		}
		else//ly remove if(base_good)
		{
			pos_first = first_seat;
		}
		PRINTF(LEVEL, "===settidiness betterseat is %d firstseatis %d===\n", pos_better, pos_first);
		if (inRuleType == 2)//2个人
		{
			swap[0] = 2;
			swap[1] = 2;
		}
		else//3个人
		{
			int pos_playe[] = { 1, 1, 1 };
			if (pos_better >= 0)
			{
				pos_playe[pos_better] = 0;
			}
			if (first_seat >= 0 && pos_playe[first_seat] == 1)
			{
				pos_playe[first_seat] = 0;
			}
			swap[0] = get_pos(pos_playe, 3);
			swap[1] = get_pos(pos_playe, 3);
		}
	}
	if(!verify_game(game, inRuleType)) {
		free(game);
        return 0;
    }

	if (inRuleType == 0 || inRuleType == 3){
		if(pos_better >= 0){
			settidiness(game, pos_better, swap[0],first_seat);
		}
		if(pos_first>=0){
			settidiness(game, pos_first, swap[1],first_seat);
		}
		game->hun= -1;
	}else if (inRuleType == 2){
		if (pos_better >= 0){
			settidiness_two(game, pos_better, swap[0], first_seat);
		}
		if (pos_first >= 0){
			settidiness_two(game, pos_first, swap[1], first_seat);
		}
		game->hun = -1;
	}else{

		if(pos_better >= 0){
			settidiness_laizi(game, pos_better, swap[0],first_seat);
		}
		if(pos_first>=0){
			settidiness_laizi(game, pos_first, swap[1],first_seat);
		}
	}

	if(game->hun==-1 && inRuleType == 1){
		game->hun = rand()%(P2+1);	
	}
	*Laizi = game->hun;
	PRINTF(LEVEL,"\n After settidiness,laizi is %d\n",game->hun);

	if(!verify_game(game, inRuleType)) {
		free(game);
		return 0;
	}

	if(0) {
		PRINTF(LEVEL,"++after set tidiness++\n");
		dumpGame(game);             //lydebug
	}
	
	if(cardReq.base_good)
	{
		// if(!cardReq->card_tidiness) //try to give good poker to the first caller
		// {

		// }
		setGoodPot(game,first_pos);//only for the first pos
	}
	if(0) {
		PRINTF(LEVEL,"++after setGoodPot++\n");
		dumpGame(game);
	}

	if(inRuleType == 1) //laizi
	{
		//check laizi for better
		if(better_pos<3)
		{
			POKERS * h = &game->other_pokers[3];
			int laizi_from_pot = 0;
			if(h->hands[(int)(h->begin)] >= 2){
				laizi_from_pot=h->begin;
			}else{		
				for (int i=h->begin+1; i<=h->end; i++)
				{
					if (h->hands[i]>0)
					{
						laizi_from_pot = i;
						break;	
					}
				}    
			}
			if(laizi_from_pot>=P2){
				laizi_from_pot=P3;
			}else{
				laizi_from_pot++;
			}
			*Laizi = laizi_from_pot;
			PRINTF(LEVEL,"++laizi_from_pot=%d,better_pos=%d,first_seat=%d++\n",laizi_from_pot,better_pos,first_seat);
			//check bettet_seat 
			if(game->other_pokers[better_pos].hands[laizi_from_pot]<2 &&
				game->bombs[better_pos].hands[laizi_from_pot]==0)
			{
				lordGame *g=game;
				POKERS all,sbomb[3],play_card[3],tmp;
				clear_poker(&all);
				for (int i=0; i<4; i++){
					add_poker( &all,&g->other_pokers[i], &all);
				}
				g->hun =laizi_from_pot;
				int pos=better_pos;
				{
					clear_poker(&sbomb[pos]);
					play_card[pos] = g->other_pokers[pos];
					//if(pos==first_seat)
					//{
					//	add_poker( &g->other_pokers[3],&play_card[i], &play_card[i]);
					//}
					play_card[pos].hun=g->hun;
					if(play_card[pos].hands[g->hun]>0)   //ly510
					{
						for(int k=0; k<MAX_POKER_KIND; k++)
						{
							if((play_card[pos].hands[k]==3)&&( k!=g->hun))
							{
								sbomb[pos].hands[k]=3;
								sbomb[pos].hands[g->hun]++;
								sbomb[pos].total = 4;
								sort_poker(&sbomb[pos]);
								sub_poker(&play_card[pos],&sbomb[pos],&play_card[pos]);
								sub_poker(&all,&sbomb[pos],&all);
								sort_poker(&all);
								k=MAX_POKER_KIND;
							}
						}
					}
					sub_poker( &all,&play_card[pos], &tmp);
					search_combos_in_pokers_with_laizi(&play_card[pos], &tmp,
						g->cs[pos], &g->smry[pos],g->hun);
					int swap=-1,swap1=-1;
					int num =2;
					int type=2;
					if ( g->smry[pos].pairs_num >0 )
					{
                        swap1=
                            swap=  g->smry[pos].pairs[0]->low;
						if(g->other_pokers[pos].hands[swap]<2){
							num = g->other_pokers[pos].hands[swap];
						}else {  
                            num=2;
						}
					}
					else if ( g->smry[pos].singles_num >1 && g->smry[pos].singles[1]->low<=P2)
					{
                        swap  =  g->smry[pos].singles[0]->low;
                        swap1 = g->smry[pos].singles[1]->low;
						num=0;
						if(g->other_pokers[pos].hands[swap]>=1){
							num ++;
						}
                        if(g->other_pokers[pos].hands[swap1]>=1){
							num ++;					
						}							
						type =11;
					}
					else
					{
						type=2;
						{
							for(int i=P3; i<=Pa; i++)
							{
								if(g->other_pokers[better_pos].hands[i]==1)
								{
									swap=i;
									num=1;
									type=1;
									break;
								}
							}
							for(int i=swap+1; i<=Pa; i++)
							{
								if(g->other_pokers[better_pos].hands[i]==1)
								{
									swap1=i;
									num=2;
									type=11;
									break;
								}
							}
						}
						if(type==2)
						{
							for(int i=P3; i<=Pa; i++)
							{
								if(g->other_pokers[better_pos].hands[i]==2) {
									swap1=swap=i;
									type = 4;
									break;
								}
							}
							if(type==2){
								num=0;
							}
						}
					}
					if(num>0)
                    {
                        for(int i=0; i<3&& num>0; i++)
						{
							if( i==better_pos) {continue;}
							if (g->other_pokers[i].hands[laizi_from_pot]!=0)
							{
								g->other_pokers[i].hands[laizi_from_pot]--;
								g->other_pokers[better_pos].hands[laizi_from_pot]++;
								g->other_pokers[i].hands[swap]++;
								g->other_pokers[better_pos].hands[swap]--;
								num--;
								swap=swap1;
							}
						}
					}
					if(num>0)
					{
						for(int i=0; i<3 && num>0; i++)
						{
							if( i==better_pos) {continue;}
							if (g->other_pokers[i].hands[laizi_from_pot]!=0)
							{
								g->other_pokers[i].hands[laizi_from_pot]--;
								g->other_pokers[better_pos].hands[laizi_from_pot]++;
								g->other_pokers[i].hands[swap]++;
								g->other_pokers[better_pos].hands[swap]--;
								num--;
								swap=swap1;
							}
						}
					}
					if(num>0)
					{
						for(int i=0; i<3 && num>0; i++)
						{
							if( i==better_pos) {continue;}
							if (g->bombs[i].hands[laizi_from_pot]!=0)
							{
								g->bombs[i].hands[laizi_from_pot]-=2;
								g->other_pokers[better_pos].hands[laizi_from_pot]+=2;
								g->other_pokers[i].hands[swap]+=1;
								g->other_pokers[better_pos].hands[swap]-=1;
								g->other_pokers[i].hands[swap1]+=1;
								g->other_pokers[better_pos].hands[swap1]-=1;
								num-=2;
								break;
							}
						}
					}
				}        
			}
		}
	}


	if(!verify_game(game, inRuleType))
	{
		free(game);
		return 0;
	}

	if(0){
		PRINTF(LEVEL,"++after inRuleType == 1++ laizi=%d\n",game->hun);
		dumpGame(game);
	}

	for(int i=0; i< 3 ; i++) {
		add_poker( &g->bombs[i],&g->other_pokers[i],
			&g->other_pokers[i]);
	}
	//change pokers to int
	convert_to_int(game,Card);
	free(game);
	return 1;
}
//#endif

int  arrange_poker_request(Request* req, int *card_list, int laizi)
{
	//g_nBombMultiple
	//g_nBetterSeat
	//g_nCardTidiness
	//g_nBaseGood
	Request res;
	//int k=0;
	//srand((unsigned)time(NULL)); 
	bool ret = GetRandomParams(&res,req->bomb_multiple,req->better_seat,req->card_tidiness,req->base_good);
	if( ret == true )
	{
		res.call_lord_first = req->call_lord_first; //???
		if (req->better_seat == 100)
		{
			res.better_seat = 0;
		}

		ret = InitCardRandom(res,(laizi == -1 ?false:true),card_list,&laizi);
	}

	return laizi;
}

#ifdef __INIT_DEBUG

#ifdef _INIT_TEST 

BOOL test_InitCardRandom(Request* cardReq, INT32 *cardList,int is_laizi)
{
	static int id=0;
	PRINTF(LEVEL,"\n============= NO. %d lordGame start==============\n",id++);
	PRINTF(LEVEL,"bomb_multiple is %d\n",cardReq->bomb_multiple);
	PRINTF(LEVEL,"card_tidiness is %d\n",cardReq->card_tidiness);
	PRINTF(LEVEL,"base_good is %d\n",cardReq->base_good);
	PRINTF(LEVEL,"better_seat is %d\n",cardReq->better_seat);
	PRINTF(LEVEL,"call_lord_first is %d\n", cardReq->call_lord_first);
	int laizi;
	int card_list[54];
	int res=InitCardRandom(*cardReq,is_laizi, card_list,&laizi);
	if (!res)
	{
		PRINTF(LEVEL,"ERROR, Init card failed\n");
		return res;
	}


	PRINTF(LEVEL,"\n=========== NO. %d Result===========\n",id-1);

	PRINTF(LEVEL,"bomb_multiple is %d\n",cardReq->bomb_multiple);
	PRINTF(LEVEL,"card_tidiness is %d\n",cardReq->card_tidiness);
	PRINTF(LEVEL,"base_good is %d\n",cardReq->base_good);
	PRINTF(LEVEL,"better_seat is %d\n",cardReq->better_seat);
	PRINTF(LEVEL,"call_lord_first is %d\n", cardReq->call_lord_first);
	{
		//  PRINTF(LEVEL,"+++++++++++++\n");
		for(int i=0; i< 3 ; i++) {
			PRINTF(LEVEL,"player%d: ",i);
			for(int j=0; j< 17 ; j++) {
				print_one_poker( int_2_poker(card_list[j+i*17]));
				cardList++;
			}
			PRINTF(LEVEL,"\n");
		}
		PRINTF(LEVEL,"pot: ");
		for(int j=0; j< 3 ; j++) {
			print_one_poker( int_2_poker(card_list[j+3*17]));
			cardList++;
		}		
		PRINTF(LEVEL,"\nlaizi: %d",laizi);        
		PRINTF(LEVEL,"\n+++++++++++++\n");
	}

	PRINTF(LEVEL,"==============lordGame end==================\n");
	return res;

}

#ifndef __INIT_DEBUG
FILE* logfile;
#endif

int main(int argc, char** argv)
{
	time_t t;
	t=time(NULL);
	logfile=fopen("result.txt","w");
	Request req= {0};
	INT32* cardlist=NULL;

	int select =3;
_restart:
	select=1;
	if(cardlist==NULL)
		cardlist=(INT32*)malloc(sizeof(INT32)*54);
	int is_laizi=1;
	printf("input laizi: \n");
	scanf("%d",&is_laizi);


	printf("1. random test for 100 games\n");
	printf("2. stress test :\n");
	printf("3. user define test\n");
	printf("4. quit\n");
	printf("select: ");
	scanf("%d",&select);
	if(select ==4) {
		free(cardlist);
		return 0;
	}
	if( select!=3) {

		int total=100;
		if (select==2) {
			printf("test how many games: ");
			scanf("%d",&total);
		}
		int t=1418850260;//1395999000;

		for(int i=0; i<total; i++)
		{
			/*  int prob_bomb[]= {455, // no bombs
			398,// 1 bombs
			122,
			21,
			2,
			1,
			1,
			1,// 5+ bombs
			};*/

			int prob_bomb[]= {255, // no bombs
				298,// 1 bombs
				222,
				121,
				102,
				1,
				1,
				1,// 5+ bombs
			};

			srand(t);
			int multi= 1<<(get_pos(prob_bomb,8));   //for laizi    
			switch(multi)
			{
			case 1:
				req.bomb_multiple=4;
				break;
			case 2:
				req.bomb_multiple=16;
				break;
			case 4:
				req.bomb_multiple=64;
				break;
			case 8:
				req.bomb_multiple=256;
				break;
			case 16:
				req.bomb_multiple=1024;
				break;
			}

			req.better_seat =rand()%4;
			req.card_tidiness = 1;
			req.base_good = rand()%2;
			req.call_lord_first = rand()%3;

			printf("game %d from seed %d\n",i,t++);
			test_InitCardRandom(&req, cardlist,is_laizi);
		}
		time_t t1=time(NULL);
		//    printf("time elapsed: %ld\n\n", (t1 -t));

	}
	else
	{
		int seed;
		printf("bomb_multiple: ");
		scanf("%d",&req.bomb_multiple);
		printf("card_tidiness: ");
		scanf("%d",&req.card_tidiness);
		printf("base_good: ");
		scanf("%d",&req.base_good);
		printf("better_seat: ");
		scanf("%d",&req.better_seat);
		printf("call_lord_first: ");
		scanf("%d",&req.call_lord_first);
		printf("seed of rand: ");
		scanf("%d",&seed);

		srand(seed);
		test_InitCardRandom(&req, cardlist,is_laizi);
	}
	goto _restart;	
	free(cardlist);
	cardlist=NULL;	
	fclose(logfile);
	return 0;
}

#endif //_INIT_TEST

#endif //_INIT_DEBUG



