#include "stdafx.h"
#include "DebugLog.h"
#include "Pokers.h"


void full_poker(POKERS* h)
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		h->hands[i]  = i>=LIT_JOKER?1:4;
	}
	h->end=BIG_JOKER;
	h->begin=P3;
	h->total=54;
}


int sort_poker(POKERS * h)
{
	int res=true;
	h->begin=-1;
	h->end=h->total=0;
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if (h->hands[i]>0)
		{
			if (h->begin==-1) h->begin = i;
			h->end =i;
		}
		else if (h->hands[i]<0)
		{
			res=false;
			h->hands[i]=0; //reset it to zero..
			PRINTF_ALWAYS("ERR in sort_poker: fatal, poker %d is negative\n",i);
		}
		h->total+=h->hands[i];
	}
	if (h->begin==-1) h->begin = 0;
	return res;
	//if (h->end>12) h->end = 12;
}

void add_poker(POKERS* a, POKERS * b, POKERS * c ) // c=a+b
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		c->hands[i]  = a->hands[i] + b->hands[i];
	}
	c->total=a->total+b->total;
	c->end=max(a->end,b->end);
	c->begin=min(a->begin,b->begin);
	sort_poker(c);
}

void sub_poker(POKERS* a, POKERS * b, POKERS * c ) // c=a-b
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		c->hands[i]  = a->hands[i] - b->hands[i];
	}
	sort_poker(c);
}

int is_sub_poker(POKERS* a, POKERS * b ) // a in b ? 1 : 0
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if ( a->hands[i] > b->hands[i]) return false;
	}
	return true;
}

int cmp_poker(POKERS* a, POKERS * b ) // return a!=b
{
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		if( a->hands[i] != b->hands[i])
			return 1;
	}
	return 0;
}

char poker_to_char(int i)
{
	switch ( i) {
	case 0:
		return '3';
	case 1:
		return '4';
	case 2:
		return '5';
	case 3:
		return '6';
	case 4:
		return '7';
	case 5:
		return '8';
	case 6:
		return '9';
	case 7:
		return 'T';
	case 8:
		return 'J';
	case 9:
		return 'Q';
	case 10:
		return 'K';
	case 11:
		return 'A';
	case 12:
		return '2';
	case 13:
		return 'X';
	case 14:
		return 'D';
	default:
		return '?';
	}
}

int  int_2_poker(int i)
{
	if (i>51)
		i-=39;
	else
		i=i%13;
	return i;
}

int get_lowest_controls( POKERS *h,int number )
{
	int high = h->end;
	if ( h->total <= number)
		return h->begin;
	//int sum=0,j=0,j1=0;
	int i;
	for ( i= high; i>0 && number>0; i--)
	{
		number -=h->hands[i];
		if (number<=0)
			return i;
	}
	return 0;
}

int get_pos_char(char *remain,int total)
{
	int sum=0;
	for(int i=0; i<total; i++)
		sum+=remain[i];
	if(sum==0)
		return -1;
	sum =rand()%sum;

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

int rand_a_poker(POKERS *all)
{
	int k;
	//    while (1)
	{
		k= get_pos_char(all->hands,MAX_POKER_KIND);
		//rand()%MAX_POKER_KIND;
		if (all->hands[k]>0)
		{
			all->hands[k]--;
			return k;
		}
		else
			return 0;
	}
}

int calc_controls( POKERS *h, POKERS *opp ,int number )
{
	int high = opp->end > h->end ? opp->end : h->end;
	int /*sum=0,*/j=0,j1=0,HighPokers1[9], HighPoker[9];
	for ( int i= high; i>0 && (j+j1)< number ; i--)
	{
		for (int k=0; k<h->hands[i]; k++)
		{
			HighPoker[j++] = i;
		}
		for (int k=0; k<opp->hands[i]; k++)
		{
			HighPokers1[j1++] = i;
		}
	}
	number = j+j1;
	j--;
	j1--;


	//j;j1;
	int M=0,N=0;
	//int bak_j=j,bak_j1=bak_j1;
	int last;
	for ( ; j >= 0 && j1>=0;)
	{
		// simulate round
		last = HighPoker[j];
		j--;
		//DBG(    print_one_poker(last);  PRINTF(LEVEL,"\n");      )
search_0:
		for (int k=j1; k>=0; k--)
			if ( HighPokers1[k]> last)
			{
				last =HighPokers1[k];
				// DBG(    print_one_poker(last);  PRINTF(LEVEL,"\n");      )
				for (int p=k; p<j1 ; p++) //remove poker
					HighPokers1[p]= HighPokers1[p+1];
				j1--;
				goto search_1;
			}
			N++;
			continue;
search_1:
			for (int k=j; k>=0; k--)
				if ( last < HighPoker[k])
				{
					last=HighPoker[k];
					// DBG(    print_one_poker(last);  PRINTF(LEVEL,"\n");      )
					for (int p=k; p<j ; p++) //remove poker
						HighPoker[p]= HighPoker[p+1];
					j--;
					goto search_0;
				}
				M++;
	}
	j=j1>=0?j:j+1;
	if (j<0) j++;
	//    PRINTF(LEVEL,"N %d M %d j %d\n", N,M,j);
	return ((N+j)*10 + M - number/2);
}

BOOL getBomb(POKERS* h, COMBO_OF_POKERS * p) //
{
	//int total=h->total;
	p->len = 1;
	for (int i=h->begin; i<=h->end; i++)
	{
		if (h->hands[i] >= 4) //bomb got
		{
			p->type = BOMB;
			p->three_desc[0]=0;
			p->low = i;
			return true;
		}
	}

	if (h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
	{
		p->type = ROCKET;
		p->low = LIT_JOKER;
		return true;
	}
	return false;
}


BOOL getThree(POKERS* h, COMBO_OF_POKERS * p) //
{
	//int total=h->total;
	for (int i=h->begin; i<=h->end; i++)
	{
		if (h->hands[i] == 3) //Got Three
		{
			p->type = THREE;
			p->low  = i;
			return true;
		}
	}
	return false;
}



BOOL getThreeSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	for (int i = h->begin; i<= (CEIL_A(h->end)- 1);)
	{
		int j;
		for (j=i; j<=CEIL_A(h->end); j++) {
			if ( h->hands[j]<3) {
				break;
			}
		}
		if ( j > i+1 )
		{
			p->type = THREE_SERIES;
			p->low = i;
			p->len = j-i;
			return true;
		}

		i=j+1;

	}
	return false;
}

BOOL getDoubleSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	for (int i = h->begin; i<= (CEIL_A(h->end) - 2);)
	{
		int j;
		for (j=i; j<=CEIL_A(h->end); j++) {
			if ( h->hands[j]<2) {
				break;
			}
		}
		if ( j >i+2  )
		{
			p->type = PAIRS_SERIES;
			p->low = i;
			p->len = j-i;
			return true;
		}
		i=j+1;
	}
	return false;
}

//Ë«Ë³µ÷ÓÅ
BOOL updateDoubleSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	//from head to tail.
	while ( p->len >3)
	{
		int end=p->len+p->low-1;
		if ( h->hands[end] > h->hands[(int)(p->low)] )
		{   //remove tail
			h->hands[end]+=2;
			p->len--;
			//end--;
		}
		else if (h->hands[(int)(p->low)] >0)
		{
			h->hands[(int)(p->low)]+=2;
			p->low++;
			p->len--;
		}
		else
			break;
	}
	return false;
}

BOOL updateDoubleSeries1(POKERS* h, COMBO_OF_POKERS * p)
{
	if (p->len ==3)
	{
		if ( h->hands[(int)(p->low)]==1 && h->hands[(int)(p->low+2)]==1) // cl
			return true;
	}
	return false;
}

BOOL getSeries(POKERS* h, COMBO_OF_POKERS * p)
{
	for (int i = h->begin; i<= (h->end - 4);)
	{
		int j;
		for (j=i; j<=CEIL_A(h->end); j++) {
			if ( h->hands[j]<1) {
				break;
			}
		}
		if ( j > i+4  )
		{
			p->type = SINGLE_SERIES;
			p->low = i;
			p->len = j-i;
			//			p->number = len;
			return true;
		}

		i=j+1;
	}
	return false;
}

BOOL getBigBomb(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * a) //
{
	//int total=h->total;
	for (int i=a->low+1; i<=h->end; i++)
	{
		if (h->hands[i] == 4) //bomb got
		{
			p->type = BOMB;
			p->three_desc[0]=0;			
			p->low = i;
			return true;
		}
	}
	if (a->low<LIT_JOKER && h->hands[LIT_JOKER]==1 &&h->hands[BIG_JOKER]==1)
	{
		p->type = ROCKET;
		p->low = LIT_JOKER;
		return true;
	}
	return false;
}

int combo_2_poker(POKERS* hand, COMBO_OF_POKERS * h) // h must be contained by hand.
{
	for(int i=P3; i<=BIG_JOKER; i++)
		hand->hands[i]=0;
	switch (h->type)
	{
	case ROCKET :
		hand->hands[LIT_JOKER]=1;
		hand->hands[BIG_JOKER]=1;
		break;
	case SINGLE_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) hand->hands[i]=1;
		break;
	case PAIRS_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) {
			hand->hands[i]=2;
		}
		break;
	case THREE_SERIES:
		for (int i=h->low; i<h->low+h->len; i++) {
			hand->hands[i]=3;
		}
		break;
	case BOMB:
		hand->hands[(int)(h->low)]=4;
		break;
	case THREE:
		hand->hands[(int)(h->low)]=3;
		break;
	case PAIRS:
		hand->hands[(int)(h->low)]=2;
		break;
	case SINGLE:
		hand->hands[(int)(h->low)]=1;
		break;
	case 31: //31
		for (int i=h->low; i<h->low+1; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])]=1;
		break;
	case 32: //32
		for (int i=h->low; i<h->low+1; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])]=2;
		//hand->hands[h->three_desc[0]];
		break;
	case 3311: //3311
		for (int i=h->low; i<h->low+2; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])]+=1;
		hand->hands[(int)(h->three_desc[1])]+=1;
		break;
	case 3322: //3322
		for (int i=h->low; i<h->low+2; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])] += 1;
		hand->hands[(int)(h->three_desc[1])] += 1;
		hand->hands[(int)(h->three_desc[0])] += 1;
		hand->hands[(int)(h->three_desc[1])] += 1;
		break;
	case 333111: //333111
		for (int i=h->low; i<h->low+3; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])] += 1;
		hand->hands[(int)(h->three_desc[1])] += 1;
		hand->hands[(int)(h->three_desc[2])] += 1;
		break;
	case 333222: //333222
		for (int i=h->low; i<h->low+3; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])] += 2;
		hand->hands[(int)(h->three_desc[1])] += 2;
		hand->hands[(int)(h->three_desc[2])] += 2;
		break;
	case 33332222: //33332222
		for (int i=h->low; i<h->low+4; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])] += 2;
		hand->hands[(int)(h->three_desc[1])] += 2;
		hand->hands[(int)(h->three_desc[2])] += 2;
		hand->hands[(int)(h->three_desc[3])] += 2;
		break;
	case 33331111: //33331111
		for (int i=h->low; i<h->low+4; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])] += 1;
		hand->hands[(int)(h->three_desc[1])] += 1;
		hand->hands[(int)(h->three_desc[2])] += 1;
		hand->hands[(int)(h->three_desc[3])] += 1;
		break;
	case 531: //531
		for (int i=h->low; i<h->low+5; i++) hand->hands[i]=3;
		hand->hands[(int)(h->three_desc[0])] += 1;
		hand->hands[(int)(h->three_desc[1])] += 1;
		hand->hands[(int)(h->three_desc[2])] += 1;
		hand->hands[(int)(h->three_desc[3])] += 1;
		hand->hands[(int)(h->three_desc[4])]+=1;
		break;
	case 411: //411
		hand->hands[(int)(h->low)]=4;
		hand->hands[(int)(h->three_desc[0])] += 1;
		hand->hands[(int)(h->three_desc[1])] += 1;
		break;
	case 422: //422
		hand->hands[(int)(h->low)]=4;
		hand->hands[(int)(h->three_desc[0])]+=2;
		hand->hands[(int)(h->three_desc[1])]+=2;
		break;
	case NOTHING:
		//break;
	default:
		PRINTF_ALWAYS("line %d, ERR, remove failed, this combo type %d not supported\n", __LINE__,h->type);
		break;
	};

	return sort_poker(hand);

}
//return hun numbers
int sub_poker_with_hun(POKERS* a, POKERS * b, POKERS * c ) // c=a-b
{
	int hun_num=0;
	c->hun = a->hun;
	for (int i=0; i<MAX_POKER_KIND; i++)
	{
		c->hands[i]  = a->hands[i] - b->hands[i];
		if(c->hands[i] <0) //carry from hun
		{
			hun_num = -c->hands[i];
			c->hands[(int)(c->hun)] +=c->hands[i];
			c->hands[i]=0;  

		}
	}
	sort_poker(c);
	return hun_num;
}
int remove_combo_poker(POKERS* hand, COMBO_OF_POKERS * h, COMBO_OF_POKERS *h1  ) // h must be contained by hand.
{
	POKERS t;
	int err= combo_2_poker(&t,h);
	if( hand->hun == -1 )
	{
		sub_poker(hand, &t, hand);
		return err;
	}
	else{
		return sub_poker_with_hun(hand, &t, hand);
	}
}
//search functions
int browse_pokers(POKERS *h, COMBO_OF_POKERS * pCombos)
{
	int num=0;
	for (int i=h->begin; i<=min(P2,h->end); i++)
	{
		if ( h->hands[i] == 0)
			continue;
		else if ( h->hands[i] == 1)
			pCombos->type = SINGLE;
		else if ( h->hands[i] == 2)
			pCombos->type = PAIRS;
		else if ( h->hands[i] == 3)
			pCombos->type = THREE;
		else if ( h->hands[i] >= 4)
			pCombos->type = BOMB;

		pCombos->low = i;
		pCombos->len = 1;
		pCombos->three_desc[0] = 0;
		//        pCombos->number = h->hands[i];
		pCombos++;
		num++;
		if ( h->hands[i] > 4)
		{
			h->hands[i]-=4;
			i--;		
		}		
	}
	//if ( h->end >= LIT_JOKER)
	//{
	if ( h->hands[LIT_JOKER]==1 && h->hands[BIG_JOKER]==1 ) {
		pCombos->type = ROCKET;
		pCombos->low= LIT_JOKER;
		pCombos->len = 1;
		pCombos++;
		num++;
	}
	else if (h->hands[LIT_JOKER]==1) {
		pCombos->type = SINGLE;
		pCombos->low = LIT_JOKER;
		pCombos->len = 1;
		pCombos++;
		num++;
	} else if (h->hands[BIG_JOKER]==1) {
		pCombos->type = SINGLE;
		pCombos->len = 1;
		pCombos->low = BIG_JOKER;
		pCombos++;
		num++;
	}
	return num;
}

//todo: add a parma number in COMBO_OF_POKERS
int get_combo_number(COMBO_OF_POKERS *h)
{
	{
		switch (h->type)
		{
		case ROCKET :
			return 2;
		case SINGLE_SERIES:
			return h->len;
		case PAIRS_SERIES:
			return h->len*2;
		case THREE_SERIES:
			return h->len*3;
		case 31: //31
		case BOMB:
			return 4;
		case THREE:
			return 3;
		case PAIRS:
			return 2;
		case SINGLE:
			return 1;

		case 32: //32
			return 5;
		case 411:
			return 6;
		case 422:
		case 3311: //3311
			return 8;
		case 3322: //3322
			return 10;
		case 333111: //333111
			return 12;
		case 333222: //333222
			return 15;
		case 33331111: //333111
			return 16;
			//	case 333222: //333222
		case 531: //333111
		case 33332222: //333222
			return 20;

		default:
			return 0;
		}
		//PRINTF(LEVEL," ");
	}
}

// BOOL getBigThree(POKERS* h, COMBO_OF_POKERS * p,COMBO_OF_POKERS* a) //
// {
// 	for (int i=a->low+1; i<=h->end; i++)
// 	{
// 		if (h->hands[i] == 3) //Got Three,doesn't check bomb..
// 		{
// 			p->type = THREE;
// 			p->low  = i;
// 			p->len = 1;
// 			return true;
// 		}
// 	}
// 	return false;
// }

// BOOL getBigSingle(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number )
// {
// 	for (int i = start ; i<= end; i++)
// 	{
// 		if (h->hands[i] >= number) //Got Three
// 		{
// 			p->type = number==3 ? THREE: (number==2 ? PAIRS: SINGLE) ;
// 			p->low  = i;
// 			p->len = 1;
// 			return true;
// 		}
// 	}
// 	return false;
// }

// BOOL getBigSeries(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number, int len ) //
// {
// 	end =CEIL_A(end);
// 	for (int i = start ; i<= (end -len + 1);)
// 	{
// 		int j;
// 		for (j=i; j<= end; j++) {
// #if 1
// 			if(number==3 && h->hands[j]!=3) break; //don't get bomb
// #endif
// 			if ( h->hands[j]<number) break;
// 		}
// 		if ( j >= i+ len )
// 		{
// 			p->type = number==3 ? THREE_SERIES: (number==2 ? PAIRS_SERIES: SINGLE_SERIES) ;
// 			p->low = i;
// 			p->len = len;
// 			//			p->number = p->len * number;
// 			return true;
// 		}
// 		i=j+1;
// 	}
// 	return false;
// }

BOOL getSingleSeries(POKERS* h, COMBO_OF_POKERS * p,int start, int end, int number ) //
{
	end =CEIL_A(end);
	for (int i = start ; i<= (end -4);)
	{
		int j;
		for (j=i; j<= end; j++) {
			if ( h->hands[j]<1) break;
			else if ( j-i >=4 && h->hands[j]>1)
			{
				j++;
				break;
			}
		}
		if ( j >= i+ 5 )
		{
			p->type =  SINGLE_SERIES;
			p->low = i;
			p->len = j-i;
			//  	    if( p->len > 5 )
			//			p->number = p->len * number;
			return true;
		}
		i=j+1;
	}
	return false;
}


int search222inSingleSeries(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * s)
{
	int len = p->len -5;
	int res = 0;
	if (len>=3 )
	{
		if ( h->hands[(int)(p->low+1)]>=1 && h->hands[(int)(p->low+2)]>=1 && h->hands[(int)(p->low)]>=1)
		{
			s->type = PAIRS_SERIES;
			s->len =3;
			s->low = p->low;
			p->len -=3;
			h->hands[(int)(p->low)]--;
			h->hands[(int)(p->low+1)]--;
			h->hands[(int)(p->low+2)]--;
			p->low+=3;

			//found a double series
			if (len >4 && h->hands[(int)(p->low)]>=1 )
			{
				s->len++;
				p->len--;

				h->hands[(int)(p->low)]--;
				p->low++;
				if (h->hands[p->low-1]>0) {
					h->hands[p->low-1]--;
					p->low--;
					p->len++;
					return 1;
				}
			}
			else 			if (h->hands[p->low-1]>0) {
				h->hands[p->low-1]--;
				p->low--;
				p->len++;
				return 1;
			}

			return 1;
		}
		else 	   if ( h->hands[p->low+p->len-1-1]>=1 && h->hands[p->low+p->len-1-2]>=1 && h->hands[p->low+p->len-1]>=1)
		{
			s->type = PAIRS_SERIES;
			s->len =3;
			s->low = p->low+p->len-3;

			h->hands[p->low+p->len-1]--;
			h->hands[p->low+p->len-1-1]--;
			h->hands[p->low+p->len-1-2]--;
			p->len -=3;
			//found a double series
			if (len >4 && h->hands[p->low+p->len-1]>=1 )
			{
				s->len++;
				s->low--;
				h->hands[p->low+p->len-1]--;
				p->len--;
				if (h->hands[p->low+p->len]>0) {
					h->hands[p->low+p->len]--;
					p->len++;
					return 1;
				}
			}
			else 	  		     if (h->hands[p->low+p->len]>0) {
				h->hands[p->low+p->len]--;
				p->len++;
				return 1;
			}

			return 1;
		}

	}
	return res;
}

int  search234inSingleSeries(POKERS* h, COMBO_OF_POKERS * p, COMBO_OF_POKERS * s) //tobe optimized
{
	//from head to tail.
	while ( p->len >5)
	{
		int end=p->len+p->low-1;
		if ( h->hands[(int)(end)] > h->hands[(int)(p->low)] )
		{   //remove tail
			h->hands[end]++;
			p->len--;
			//end--;
		}
		else if (h->hands[(int)(p->low)] >0)
		{
			h->hands[(int)(p->low)]++;
			p->low++;
			p->len--;
		}
		else
			break;
	}
	//check_five_series:
	//todo:
	return 0;
}


int  searchMultiSingleSeries(POKERS* h, COMBO_OF_POKERS * p) //tobe optimized
{
	//    POKERS t,*hbak;
	//    memmove(&t,h,sizefo(POKERS));
	//    h=&t;
	COMBO_OF_POKERS * first = p;
	int num=0;

	// got all 5-series
	for (int i = h->begin; i<= (CEIL_A(h->end) - 4); i++)
	{
		//        if (getBigSeries(h, p, i ,  CEIL_A(h->end), 1, 5)) //could be optimized
		if (getSingleSeries(h, p, i ,  CEIL_A(h->end), 1)) //could be optimized
		{
			remove_combo_poker(h, p, NULL);
			p++;
			num++;
		}
	}
	p=first;
	for (int i=0; i<num; i++) {
		{
			while ( p->len+p->low<=Pa && h->hands[p->len+p->low])
			{
				h->hands[p->len+p->low]--;
				p->len++;
			}
			p++;
		}
	}

	p=first;
	int num_bak =num;
	for (int i=0; i<num; i++) {

		if (  i<num-1 && p->low+p->len == (p+1)->low ) //connect
		{
			num--;
			p->len+= (p+1)->len;
			i++;
			if ( num > i )
				memmove(p+1 ,p+2,sizeof(COMBO_OF_POKERS)*(num-i));
		}
		else  if (  i<num-2 && p->low+p->len == (p+2)->low ) //connect
		{

			p->len+= (p+2)->len;
			if ( num ==4 )
				memmove(p+2 ,p+3,sizeof(COMBO_OF_POKERS)*(1));
			num--;
		}
		else if (  i<num-3 && p->low+p->len == (p+3)->low ) //connect
		{

			p->len+= (p+3)->len;
			//              if ( num ==3 )
			//                memmove(p+2 ,p+3,sizeof(COMBO_OF_POKERS)*(num-2));
			num--;
		}
		p++;
	}

	for(int i=num; i<num_bak; i++)
	{
		first[i].type = NOTHING;
	}
	return num;
}

int search_general_1(POKERS* h , COMBO_OF_POKERS * pCombos,
					 bool skip_bomb,bool skip_double, bool skip_three, bool skip_series)
{
	int combo_nums=0;
	int num =0;
	if (!skip_series) {

		num= searchMultiSingleSeries(h,pCombos);
		combo_nums+=num;
		pCombos+=num;
		COMBO_OF_POKERS * tmp=pCombos-num;
		for (int k=0; k<num; k++)
		{
			int res =  search222inSingleSeries(h,tmp+k,pCombos);
			if (res==1) {
				pCombos++;
				combo_nums++;
			}
			else
				search234inSingleSeries(h,tmp+k,pCombos);
			//     remove_combo_poker(h,pCombos,NULL);
		}
	}
	if (!skip_bomb)
		while ( getBomb(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}

		if (!skip_three)
			while ( getThreeSeries(h,pCombos))
			{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
			}

			if (!skip_double)
				while ( getDoubleSeries(h,pCombos)) //todo: check three in doubles.
				{
					remove_combo_poker(h,pCombos,NULL);
					//          updateDoubleSeries(h,pCombos);
					combo_nums++;
					pCombos++;
				}

				/*   	while( getThree(h,pCombos))
				{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
				}
				*/

				//todo search for three's in series head and tail.
				num = browse_pokers(h,pCombos);
				pCombos+=num;
				combo_nums+=num;


				//  print_all_combos(pCombos-combo_nums,combo_nums);

				return combo_nums;
}

int search_general_2(POKERS* h , COMBO_OF_POKERS * pCombos,
					 bool skip_bomb,bool skip_double, bool skip_three, bool skip_series)
{
	int combo_nums=0;
	int num ;
	if (!skip_bomb)
		while ( getBomb(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}

		if (!skip_three)
			while ( getThreeSeries(h,pCombos))
			{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
			}
			if (!skip_series)	{

				num = searchMultiSingleSeries(h,pCombos);
				combo_nums+=num;
				pCombos+=num;
				COMBO_OF_POKERS * tmp=pCombos-num;
				for (int k=0; k<num; k++)
				{
					int res =  search222inSingleSeries(h,tmp+k,pCombos);
					if (res==1) {
						pCombos++;
						combo_nums++;
					}
					else
						search234inSingleSeries(h,tmp+k,pCombos);
					//     remove_combo_poker(h,pCombos,NULL);
				}
			}

			if (!skip_double)
				//may be removed.
					while ( getDoubleSeries(h,pCombos)) //todo: check three in doubles.
					{
						remove_combo_poker(h,pCombos,NULL);
						//         updateDoubleSeries(h,pCombos);
						combo_nums++;
						pCombos++;
					}

					//todo search for three's in series head and tail.
					num = browse_pokers(h,pCombos);
					pCombos+=num;
					combo_nums+=num;


					//   print_all_combos(pCombos-combo_nums,combo_nums);

					return combo_nums;
}

int search_general_3(POKERS* h , COMBO_OF_POKERS * pCombos,
					 bool skip_bomb,bool skip_double, bool skip_three, bool skip_series)
{
	int combo_nums=0;
	int num;
	if (!skip_bomb)
		while ( getBomb(h,pCombos))
		{
			remove_combo_poker(h,pCombos,NULL);
			combo_nums++;
			pCombos++;
		}

		if (!skip_three)
			while ( getThreeSeries(h,pCombos))
			{
				remove_combo_poker(h,pCombos,NULL);
				combo_nums++;
				pCombos++;
			}

			if (!skip_double)
				while ( getDoubleSeries(h,pCombos))
				{
					remove_combo_poker(h,pCombos,NULL);
					//      updateDoubleSeries(h,pCombos);
					combo_nums++;
					pCombos++;
				}

				if (!skip_series)
				{
					num = searchMultiSingleSeries(h,pCombos);
					combo_nums+=num;
					pCombos+=num;
					COMBO_OF_POKERS * tmp=pCombos-num;
					for (int k=0; k<num; k++)
					{
						int res =  search222inSingleSeries(h,tmp+k,pCombos);
						if (res==1) {
							pCombos++;
							combo_nums++;
						}
						else
							search234inSingleSeries(h,tmp+k,pCombos);
						//     remove_combo_poker(h,pCombos,NULL);
					}

				}
				//todo search for three's in series head and tail.
				num = browse_pokers(h,pCombos);
				pCombos+=num;
				combo_nums+=num;

				//    print_all_combos(pCombos-combo_nums,combo_nums);

				return combo_nums;
}

COMBO_OF_POKERS*  find_max_len_in_combos(COMBO_OF_POKERS* combos, int total)
{
	COMBO_OF_POKERS* cur=NULL;
	int	max= 0;
	for (int k=0; k<total; k++)
	{
		if (combos[k].type!=NOTHING)
		{
			int tmp;
			(tmp=get_combo_number(&combos[k]));
			if (tmp>max) {
				max=tmp;
				cur= combos+k;
			}
			else if (tmp == max && max>1 && combos[k].low < cur->low)
			{
				max=tmp;
				cur= combos+k;
			}
			else if (tmp == max && max==1 && combos[k].low > cur->low)
			{
				max=tmp;
				cur= combos+k;
			}
		}
	}
	return cur;
}

COMBO_OF_POKERS*  find_biggest_in_combos(COMBO_OF_POKERS* combos, int total)
{
	COMBO_OF_POKERS* cur=NULL;
	int	max= 0;
	for (int k=0; k<total; k++)
	{
		if (combos[k].type!=NOTHING)
		{
			if (combos[k].low>=max) {
				max=combos[k].low;
				cur= combos+k;
			}
		}
	}
	return cur;
}

/*
COMBO_OF_POKERS*  find_smallest_in_combos(COMBO_OF_POKERS* combos, int total,PLAYER* player)
{
COMBO_OF_POKERS* cur=NULL;
int	max= 0;
for (int k=0; k<total; k++)
{
if (combos[k].type!=NOTHING &&)
{int tmp;
if(combos[k].low<max){
max=tmp;
cur= combos+k;
}
else if(combos[k].low == max && get_combo_number(combos[k])< get_combo_number(cur)){
max=tmp;
cur= combos+k;
}
}
}
return cur;
}
*/

int get_control_poker_num_in_combo(COMBO_OF_POKERS* c, int lower)
{
	switch ( c->type )
	{
	case ROCKET:
		//s->singles[s->series_num]=&c[k];
		//s->rocket_num++;
		//break;
		return 0;
	case BOMB:
		//if ( c->low >=lower)
		//	return 4;
		return 0;
	case SINGLE_SERIES:
		if ( c->low + c->len - 1  >=lower)
			return c->low+c->len-lower;
	case PAIRS_SERIES:
		if ( c->low + c->len - 1  >=lower)
			return 2*(c->low+c->len-lower);
	case THREE_SERIES:
		if ( c->low + c->len - 1  >=lower)
			return 3*(c->low+c->len-lower);
	case 31:
		return 3*( c->low >=lower) + c->three_desc[0]>=lower ;
	case 32:
		return ( c->low >=lower)*3 + 2*(c->three_desc[0]>=lower);
#if 0
	case THREE:
		if ( c->low >=lower)
			return 3;
	case PAIRS :

		if ( c->low >=lower)
			return 2;
	case SINGLE :
		if ( c->low >=lower)
			return 1;
#else
	case THREE:  //donot check three,pairs,single
	case PAIRS :
	case SINGLE :
		return 0;
#endif
	case 3311:
	case 333111:
	case 33331111:
	case 531: {
		int 	num =0;
		if ( c->low + c->len - 1  >=lower)  //todo fixme...
			num = 3*(c->low+c->len-lower);
		for (int i=0; i<c->len; i++)
		{
			num += c->three_desc[i]>=lower;
		}
		return num;
			  }
	case 3322:
	case 333222:
	case 33332222:
		{
			int 	num =0;
			if ( c->low + c->len - 1  >=lower)  //todo fixme...
				num = 3*(c->low+c->len-lower);
			for (int i=0; i<c->len; i++)
			{
				num += 2*(c->three_desc[i]>=lower);
			}
			return num;
		}
	case 411:
		{
			int 	num =0;
			if ( c->low >=lower)  //todo fixme...
				num = 4;
			for (int i=0; i<2; i++)
			{
				num += (c->three_desc[0]>=lower);
			}
			return num;
		}
	case 422:
		{
			int 	num =0;
			if ( c->low >=lower)  //todo fixme...
				num = 4;
			for (int i=0; i<2; i++)
			{
				num += 2*(c->three_desc[0]>=lower);
			}
			return num;
		}
	}

	return 0;
}

/*
i.	控制 - 套的数目 最少。
ii.	套的种类最少
iii.	单牌数量最少
iv.	最小的单牌最大
*/
int cmp_summary(COMBOS_SUMMARY *a, COMBOS_SUMMARY *b) //return a>b
{
	if (	(a->combo_total_num-a->extra_bomb_ctrl< b->combo_total_num-b->extra_bomb_ctrl )
		)
		return true;
	else	if (	a->combo_total_num-a->extra_bomb_ctrl > b->combo_total_num-b->extra_bomb_ctrl
		)
		return false;
	else  if (a->combo_total_num-a->extra_bomb_ctrl == b->combo_total_num-b->extra_bomb_ctrl
		)
	{
		if(a->extra_bomb_ctrl>b->extra_bomb_ctrl)
			return true;
		if(a->extra_bomb_ctrl<b->extra_bomb_ctrl)
			return false;		
		if (a->combo_typenum < b->combo_typenum)
			return true;
		else if (a->combo_typenum > b->combo_typenum)
			return false;
		else {
			if (a->singles_num > b->singles_num )
				return false;
			else if (a->singles_num < b->singles_num )
				return true;
			else
				return a->combo_smallest_single > b->combo_smallest_single;
		}
	}
	return 0;
}