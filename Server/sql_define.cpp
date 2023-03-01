#include "stdafx.h"
#include "sql_define.h"

const char* CREATE_NORMAL_CARD_TABLE = "create table if not exists tb_normal_card(id integer primary key autoincrement,nc_player_0 varchar(255) ,nc_player_1 varchar(255) ,nc_player_2 varchar(255) ,nc_lord_card varchar(255) ,nc_double integer);";
const char* CREATE_SPECIFIC_CARD_TABLE = "create table if not exists tb_specific_card(id integer primary key autoincrement,sc_player_0 varchar(255),sc_player_1 varchar(255),sc_player_2 varchar(255),sc_lord_card varchar(255) ,sc_double integer);";
const char* CREATE_LAIZI_NORMAL_CARD_TABLE = "create table if not exists tb_laizi_normal_card ( id integer primary key autoincrement,ln_player_0 varchar(255),ln_player_1 varchar(255),ln_player_2 varchar(255),ln_lord_card varchar(255),ln_double integer);";
const char* CREATE_LAIZI_SPECIFIC_CARD_TABLE = "create table if not exists tb_laizi_specific_card(id integer primary key autoincrement,ls_player_0 varchar(255),ls_player_1 varchar(255),ls_player_2 varchar(255),ls_lord_card varchar(255),ls_double integer);";

const char* UPDATE_NORMAL_CARD_TABLE = "insert into tb_normal_card(nc_player_0,nc_player_1,nc_player_2,nc_lord_card,nc_double) values('%s','%s','%s','%s',%d);";
const char* UPDATE_SPECIFIC_CARD_TABLE = "insert into tb_specific_card(sc_player_0,sc_player_1,sc_player_2,sc_lord_card,sc_double) values('%s','%s','%s','%s',%d);";
const char* UPDATE_LAIZI_NORMAL_CARD_TABLE = "insert into tb_laizi_normal_card(ln_player_0,ln_player_1,ln_player_2,ln_lord_card,ln_double) values('%s','%s','%s','%s',%d);";
const char* UPDATE_LAIZI_SPECIFIC_CARD_TABLE = "insert into tb_laizi_specific_card(ls_player_0,ls_player_1,ls_player_2,ls_lord_card,ls_double) values('%s','%s','%s','%s',%d);";

const char* QUERY_NORMAL_CARD_TABLE = "select * from tb_normal_card order by id desc limit 1";
const char* QUERY_SPECIFIC_CARD_TABLE = "select * from tb_specific_card where sc_double >= %d order by sc_double desc limit 1000";
const char* QUERY_LAIZI_NORMAL_CARD_TABLE = "select * from tb_laizi_normal_card order by id desc limit 1";
const char* QUERY_LAIZI_SPECIFIC_CARD_TABLE = "select * from tb_laizi_specific_card where ls_double <= %d order by ls_double desc limit 1";

const char* QUERY_DOUBLE_LAIZI_SPECIFIC_CARD_TABLE = "select * from tb_laizi_specific_card where ls_double = %d order by id desc limit 50";
const char* QUERY_DOUBLE_SPECIFIC_CARD_TABLE = "select * from tb_specific_card where ts_dobule = %d order by id desc limit 50";

const char* TABLE_NORMAL_CARD = "tb_normal_card";
const char* TABLE_SPECIFIC_CARD = "tb_specific_card";
const char* TABLE_LAIZI_NORMAL_CARD = "tb_laizi_normal_card";
const char* TABLE_LAIZI_SPECIFIC_CARD = "tb_laizi_specific_card";