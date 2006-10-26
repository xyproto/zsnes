// ST-011 SNES DSP adapted from Morita Shogi 64
//
// notes:
// - the SNES uses DMA to/from 60:0000 and maybe 68:0xxx
// - some code redundancy (helper subroutines for movement projection)
//
// - OPS04/05 have unknown output values (!)
// - OPS06/07 have unknown purposes
//
// - plenty of missing opcodes that don't show up in the only known binary log (st011-demo)
//   (play the game until captured/promoted pieces, king checked, endgame)
// - minus emulation cycles (busy signals), bit-perfect to 'st011-demo'

//#define DEBUG_DSP

#ifdef DEBUG_DSP
#include <stdio.h>
int debug1, debug2;
int line_count;
#endif


void (*RunST011)();
void ST011_Command();

unsigned char ST011_DR;
unsigned char ST011_SR;

int ST011_input_length;

#define ST011_ram setaramdata

extern unsigned char *setaramdata;

#define ST011_board ( ST011_ram+0x130 )

int ST011_dma_count;
int ST011_dma_index;

int ST011_king1;
int ST011_king2;

// (x,y)
#define MOVE_UUL   -1,-20
#define MOVE_UL		 -1,-10
#define MOVE_ULAll -9,- 9
#define MOVE_U		  0,-10
#define MOVE_UAll   0,- 9
#define MOVE_UR		  1,-10
#define MOVE_URAll  9,- 9
#define MOVE_UUR    1,-20

#define MOVE_L		 -1,  0
#define MOVE_LAll	 -9,  0
#define MOVE_R		  1,  0
#define MOVE_RAll	  9,  0

#define MOVE_DDL	 -1, 20
#define MOVE_DL		 -1, 10
#define MOVE_DLAll -9,  9
#define MOVE_D		  0, 10
#define MOVE_DAll   0,  9
#define MOVE_DR		  1, 10
#define MOVE_DRAll  9,  9
#define MOVE_DDR	  1, 20

#define MOVE_STOP 127,127
#define MOVE_NOP	  0,  0
const int ST011_move_table[8*2][9*2] =
{
	// Pawn: one step forward
	// - Promoted: same as Gold
	{ MOVE_D, MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },
	{ MOVE_DR, MOVE_D, MOVE_DL, MOVE_R, MOVE_L, MOVE_U, MOVE_STOP, MOVE_NOP, MOVE_NOP },

	// Lance: all steps forward
	// - Promoted: same as Gold
	{ MOVE_DAll, MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },
	{ MOVE_DR, MOVE_D, MOVE_DL, MOVE_R, MOVE_L, MOVE_U, MOVE_STOP, MOVE_NOP, MOVE_NOP },

	// Knight: one step side, two forward
	// - Promoted: same as Gold
	{ MOVE_DDR, MOVE_DDL, MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },
	{ MOVE_DR, MOVE_D, MOVE_DL, MOVE_R, MOVE_L, MOVE_U, MOVE_STOP, MOVE_NOP, MOVE_NOP },

	// Silver general: one any diagonal, one step forward
	// - Promoted: same as Gold
	{ MOVE_DR, MOVE_D, MOVE_DL, MOVE_UR, MOVE_UL, MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },
	{ MOVE_DR, MOVE_D, MOVE_DL, MOVE_R, MOVE_L, MOVE_U, MOVE_STOP, MOVE_NOP, MOVE_NOP },

	// Gold general: one any forward, one sideways or one backward
	// - Promoted: N/A
	{ MOVE_DR, MOVE_D, MOVE_DL, MOVE_R, MOVE_L, MOVE_U, MOVE_STOP, MOVE_NOP, MOVE_NOP },
	{ MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },

	// Bishop: any diagonal
	// - Promoted: Bishop + King
	{ MOVE_DRAll, MOVE_DLAll, MOVE_URAll, MOVE_ULAll, MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },
	{ MOVE_DRAll, MOVE_D, MOVE_DLAll, MOVE_R, MOVE_L, MOVE_URAll, MOVE_U, MOVE_ULAll, MOVE_STOP },

	// Rook: any vertical, horizontal
	// - Promoted: Rook + King
	{ MOVE_DAll, MOVE_RAll, MOVE_LAll, MOVE_UAll, MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },
	{ MOVE_DR, MOVE_DAll, MOVE_DL, MOVE_RAll, MOVE_LAll, MOVE_UR, MOVE_UAll, MOVE_UL, MOVE_STOP },

	// King: one any direction
	// - Promoted: N/A
	{ MOVE_DR, MOVE_D, MOVE_DL, MOVE_R, MOVE_L, MOVE_UR, MOVE_U, MOVE_UL, MOVE_STOP },
	{ MOVE_STOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP, MOVE_NOP },
};


void ST011_Reset()
{
	RunST011 = &ST011_Command;
  ST011_SR=0xff;
}


void ST011_OP01_A()
{
	if( ST011_dma_count-- )
	{
		ST011_board[ ST011_dma_index++ ] = ST011_DR;
	}

	if( ST011_dma_count == 0 )
	{
#ifdef DEBUG_DSP
		int lcv1, lcv2;
#endif
		int lcv;

		for( lcv = 0; lcv < 11; lcv++ )
		{
			ST011_board[ lcv ] = 0;
		}
		for( lcv = 11; lcv < 21; lcv++ )
		{
			ST011_board[ lcv ] = 0x80;
		}

		ST011_king1 = ST011_board[ 126+21 ];
		ST011_king2 = ST011_board[ 127+21 ];

		RunST011 = &ST011_Command;
		ST011_SR = 0xc4;

#ifdef DEBUG_DSP
		// Debug
		printf( "OP01\n" );
		for( lcv1 = 0; lcv1 < 9; lcv1++ )
		{
			for( lcv2 = 0; lcv2 < 10; lcv2++ )
			{
				printf( "%02x ", ST011_board[ lcv1*10 + lcv2 + 21 ] );
			}

			printf( "\n" );
		}
		printf( "OP01 END\n\n" );
#endif
	}
}

void ST011_OP01()
{
	ST011_dma_count = 128;
	ST011_dma_index = 0+21;

	RunST011 = &ST011_OP01_A;
	ST011_SR = 0xa4;
}


void ST011_OP02_A()
{
	if( ST011_dma_count-- )
	{
		ST011_DR = ST011_ram[ ST011_dma_index-- ];
	}

	if( ST011_dma_count == 0 )
	{
#ifdef DEBUG_DSP
		int lcv1, lcv2;
#endif

		RunST011 = &ST011_Command;
		ST011_SR = 0xc4;

#ifdef DEBUG_DSP
		// Debug
#define OP02_ROW 10

		printf( "OP02\n" );
		for( lcv1 = 0; lcv1 < 0x83 / OP02_ROW; lcv1++ )
		{
			for( lcv2 = 0; lcv2 < OP02_ROW; lcv2++ )
			{
				printf( "%02x ", ST011_ram[ debug1 - lcv1 * OP02_ROW - lcv2 ] );
			}

			printf( "\n" );
		}
		printf( "OP02 END\n\n" );
#endif
	}
}

void ST011_OP02()
{
	switch( ST011_input_length-- )
	{
		case 4: ST011_dma_index = ST011_DR;	break;
		case 3: ST011_dma_index |= ST011_DR << 8;	break;
		case 2: ST011_dma_count = ST011_DR;	break;
		case 1:
			ST011_dma_count |= ST011_DR << 8;

#ifdef DEBUG_DSP
			debug1 = ST011_dma_index;
			debug2 = 0;
#endif

			RunST011 = &ST011_OP02_A;
			ST011_SR = 0xa4;
			break;
	}
}


void ST011_Project_Moves( int color )
{
	int row, col, lcv, index;
	int dir;

	index = 0x121;
	for( lcv = 0; lcv < 0x83; lcv++ )
	{
		ST011_ram[ index-- ] = 0;
	}
	index = 0x121 - 21;

	if( color == 0x20 )
	{
		dir = 1;
	}
	else
	{
		dir = -1;
	}

	for( row = 0; row < 9; row++ )
	{
		for( col = 0; col < 10; col++ )
		{
			int shogi_piece;
			int piece_id;
			int lcv_steps, lcv_move;
			int move_list[ 9*2 ];

			shogi_piece = ST011_board[ row*10+col+21 ];
			piece_id = shogi_piece & 0x1f;

			if( col == 9 ) continue;
			if( shogi_piece == 0x00 ) continue;
			if( ( shogi_piece & ~0x1f ) != color ) continue;

			for( lcv = 0; lcv < 9*2; lcv++ )
			{
				move_list[ lcv ] = ST011_move_table[ piece_id >> 1 ][ lcv ];
			}

			lcv_move = 0;
			while( move_list[ lcv_move ] != 0x7f )
			{
				int pos_x, pos_y;

				lcv_steps = 1;
				if( move_list[ lcv_move ] == 9 || move_list[ lcv_move ] == -9 )
				{
					lcv_steps = 9;
					if( move_list[ lcv_move ] == 9 )
					{
						move_list[ lcv_move ] = 1;
					}
					else
					{
						move_list[ lcv_move ] = -1;
					}
				}

				if( move_list[ lcv_move+1 ] == 9 || move_list[ lcv_move+1 ] == -9 )
				{
					lcv_steps = 9;
					if( move_list[ lcv_move+1 ] == 9 )
					{
						move_list[ lcv_move+1 ] = 1;
					}
					else
					{
						move_list[ lcv_move+1 ] = -1;
					}
				}
				else
				{
					move_list[ lcv_move+1 ] /= 10;
				}

				pos_x = col;
				pos_y = row;
				while( lcv_steps-- )
				{
					pos_x += move_list[ lcv_move+0 ];
					pos_y += ( move_list[ lcv_move+1 ] * dir );

					ST011_ram[ index - pos_y*10 - pos_x ] = 0x80;

					if( ST011_board[ pos_y*10 + pos_x + 21 ] ) break;
				}

				lcv_move += 2;
			}
		} // end col
	} // end row
}


int ST011_Project_Valid_Moves( int color )
{
	int row, col, lcv, index;
	int dir;

	index = 0x556;

	if( color == 0x20 )
	{
		dir = 1;
	}
	else
	{
		dir = -1;
	}

	for( row = 0; row < 9; row++ )
	{
		for( col = 0; col < 10; col++ )
		{
			int shogi_piece;
			int piece_id;
			int lcv_steps, lcv_move;
			int move_list[ 9*2 ];

			shogi_piece = ST011_board[ row*10+col+21 ];
			piece_id = shogi_piece & 0x1f;

			if( col == 9 ) continue;
			if( shogi_piece == 0x00 ) continue;
			if( ( shogi_piece & ~0x1f ) != color ) continue;

			for( lcv = 0; lcv < 9*2; lcv++ )
			{
				move_list[ lcv ] = ST011_move_table[ piece_id >> 1 ][ lcv ];
			}

			lcv_move = 0;
			while( move_list[ lcv_move ] != 0x7f )
			{
				int pos_x, pos_y;

				lcv_steps = 1;
				if( move_list[ lcv_move ] == 9 || move_list[ lcv_move ] == -9 )
				{
					lcv_steps = 9;
					if( move_list[ lcv_move ] == 9 )
					{
						move_list[ lcv_move ] = 1;
					}
					else
					{
						move_list[ lcv_move ] = -1;
					}
				}

				if( move_list[ lcv_move+1 ] == 9 || move_list[ lcv_move+1 ] == -9 )
				{
					lcv_steps = 9;
					if( move_list[ lcv_move+1 ] == 9 )
					{
						move_list[ lcv_move+1 ] = 1;
					}
					else
					{
						move_list[ lcv_move+1 ] = -1;
					}
				}
				else
				{
					move_list[ lcv_move+1 ] /= 10;
				}

				pos_x = col;
				pos_y = row;
				while( lcv_steps-- )
				{
					pos_x += move_list[ lcv_move+0 ];
					pos_y += ( move_list[ lcv_move+1 ] * dir );

					if( pos_x < 0 ) break;
					if( pos_x > 8 ) break;
					if( pos_y < 0 ) break;
					if( pos_y > 8 ) break;
					if( ( ST011_board[ pos_y*10 + pos_x + 21 ] & ~0x1f ) == color ) break;

					ST011_ram[ index + 0x000 ] = 21 + row*10 + col;
					ST011_ram[ index + 0x001 ] = 0;
					ST011_ram[ index + 0x418 ] = 21 + pos_y*10 + pos_x;
					ST011_ram[ index + 0x419 ] = 0;

					if( pos_y >= 6 )
					{
						ST011_ram[ index + 0x418 ] |= 0x80;
					}
					index += 2;

					if( ST011_board[ pos_y*10+pos_x+21 ] ) break;
				}

				lcv_move += 2;
			}
		} // end col
	} // end row

	return ( index-0x556 ) >> 1;
}


void ST011_OP04()
{
	ST011_Project_Moves( 0x40 );

	// unknown outputs
	ST011_ram[ 0x12c ] = 0;
	ST011_ram[ 0x12d ] = 0;
	ST011_ram[ 0x12e ] = 0;
	ST011_ram[ 0x12f ] = 0;

	RunST011 = &ST011_Command;
	ST011_SR = 0xc4;
}


void ST011_OP05()
{
	ST011_Project_Moves( 0x20 );

	// unknown outputs
	ST011_ram[ 0x12c ] = 0;
	ST011_ram[ 0x12d ] = 0;
	ST011_ram[ 0x12e ] = 0;
	ST011_ram[ 0x12f ] = 0;

	RunST011 = &ST011_Command;
	ST011_SR = 0xc4;
}


void ST011_OP0E()
{
	int valid_moves;

	valid_moves = ST011_Project_Valid_Moves( 0x20 );

	ST011_ram[ 0x12c ] = valid_moves & 0xff;
	ST011_ram[ 0x12d ] = ( valid_moves >> 8 ) & 0xff;

	RunST011 = &ST011_Command;
	ST011_SR = 0xc4;
}


void ST011_Command()
{
#ifdef DEBUG_DSP
	printf( "OP%02X @ line %d\n", ST011_DR, line_count );
#endif

	// busy
	ST011_SR = 0x84;

	switch( ST011_DR )
	{
		// Download shogi playboard to on-board memory
		case 0x01:
			ST011_OP01();
			break;

		// Upload shogi analysis data to outside memory
		case 0x02:
			ST011_input_length = 4;
			RunST011 = ST011_OP02;
			break;

		// Project all moves of player color $40
		case 0x04:
			ST011_OP04();
			break;

		// Project all moves of player color $20
		case 0x05:
			ST011_OP05();
			break;

		// Unknown - seems to set flags $00,$20,$40,..$e0 for restricted movement lists
		case 0x06:
			//ST011_OP06();
			ST011_SR = 0xc4;
			break;

		// Unknown - seems to set flags $00,$20,$40,..$e0 for restricted movement lists
		case 0x07:
			//ST011_OP07();
			ST011_SR = 0xc4;
			break;

		// List valid moves of player color $20
		case 0x0E:
			ST011_OP0E();
			break;

		default:
#ifdef DEBUG_DSP
			printf( "Unknown OP @ line %d\n", line_count );
#endif
			break;
	}
}


unsigned short seta11_address;
unsigned char seta11_byte;

void ST011_MapR_68()
{
  if (seta11_address < 0x1000)
  {
    ST011_DR = ST011_ram[seta11_address & 0xfff];
  }
  seta11_byte = ST011_DR;
}

void ST011_MapW_68()
{
  ST011_DR = seta11_byte;

  if (seta11_address < 0x1000)
  {
    ST011_ram[seta11_address & 0xfff] = ST011_DR;
  }
}

void ST011_MapR_60()
{
  if (seta11_address == 0)
  {
    RunST011();
  }
  if (seta11_address == 1)
  {
    seta11_byte = ST011_SR;
    return;
  }
  seta11_byte = ST011_DR;
}

void ST011_MapW_60()
{
  ST011_DR = seta11_byte;

  if (seta11_address == 0)
  {
    RunST011();
  }
}
