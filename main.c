#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "chessSystem.h"
#include "map.h"

typedef struct chess_system_t{
    Map tournaments;//key is tournament_id
    Map players;
    int num_games;
    int num_valid_time_games;
    int sum_valid_time_games;
} *ChessSystem;

typedef struct chess_tournament_t{
    Map games;
    Map players;
    int tournament_id;
    const char* tournament_location;
    Winner tournament_winner;
    int max_games_per_player;
    bool has_ended;
    int longest_game_time;
    int num_valid_time_games;
    int sum_valid_time_games;
    int num_games;
    int num_players;
} *ChessTournament;

typedef struct chess_game_t{
    int game_id;
    int first_player;
    int second_player;
    Winner game_winner;
    int play_time;
} *ChessGame;

typedef struct player{
    int id;
    int num_games;
    int num_wins;
    int num_draws;
    int num_losses;
    bool has_been_removed;
} *Player;

MapDataElement copyMapDataTournament(MapDataElement data){
    if(data == NULL){
        return NULL;
    }
    ChessTournament copy = (ChessTournament)malloc(sizeof(*copy));
    if(copy == NULL){
        return NULL;
    }
    copy->tournament_id = ((ChessTournament)data)->tournament_id;
    Map games = ((ChessTournament)data)->games;
    copy->games = mapCopy(games);
    if(copy->games == NULL){
        free(copy);
        return NULL;
    }
    Map players = ((ChessTournament)data)->players;
    copy->players = mapCopy(players);
    if(copy->players == NULL){
        mapDestroy(copy->games);
        free(copy);
        return NULL;
    }
    copy->tournament_location = ((ChessTournament)data)->tournament_location;
    copy->tournament_winner = ((ChessTournament)data)->tournament_winner;
    copy->max_games_per_player = ((ChessTournament)data)->max_games_per_player;
    copy->has_ended = ((ChessTournament)data)->has_ended;
    copy->longest_game_time = ((ChessTournament)data)->longest_game_time;
    copy->num_valid_time_games = ((ChessTournament)data)->num_valid_time_games;
    copy->sum_valid_time_games = ((ChessTournament)data)->sum_valid_time_games;
    copy->num_games = ((ChessTournament)data)->num_games;
    copy->num_players = ((ChessTournament)data)->num_players;
    return copy;
}

MapDataElement copyMapDataGame(MapDataElement data) {
    ChessGame copy = (ChessGame) malloc(sizeof(*copy));
    if (data == NULL) {
        return NULL;
    }
    copy->game_id = ((ChessGame) data)->game_id;
    copy->first_player = ((ChessGame) data)->first_player;
    copy->second_player = ((ChessGame) data)->second_player;
    copy->game_winner = ((ChessGame) data)->game_winner;
    copy->play_time = ((ChessGame) data)->play_time;
    return copy;
}

MapDataElement copyMapDataPlayers(MapDataElement data) {
    Player copy = (Player) malloc(sizeof(*copy));
    if (data == NULL) {
        return NULL;
    }
    copy->id = ((Player) data)->id;
    copy->num_games = ((Player) data)->num_games;
    copy->num_wins = ((Player) data)->num_wins;
    copy->num_draws = ((Player) data)->num_draws;
    copy->num_losses = ((Player) data)->num_losses;
    copy->has_been_removed = ((Player) data)->has_been_removed;
    return copy;
}

//TODO: create map
ChessSystem chessCreate(){
    ChessSystem Chess = (ChessSystem)malloc(sizeof(struct chess_system_t));
    if(Chess==NULL)
        return NULL;
    Map tournaments_map = mapCreate();
    Map players_map = mapCreate();
    Chess->tournaments=tournaments_map;
    Chess->players=players_map;
    Chess->num_games=0;
    Chess->num_valid_time_games=0;
    Chess->sum_valid_time_games=0;
};

//TODO: check what needs to be freed
void chessDestroy(ChessSystem chess){
    if(chess == NULL)
        return;
    //free(chess)
};

//check if ID is a positive number
static bool isValidID(int id)
{
    if(id < 0)
        return false;
    return true;
}

//check if location is valid string by specified conditions
static bool isValidLocation(const char* location)
{
    char* p=location;
    if(p == NULL)
        return false;
    if(*p >'Z' || *p < 'A')//check if first letter is capital
        return false;
    p++;
    while(*p)
    {
        if(*p != ' ' && (*p < 'a' || *p >'z'))//check if remaining letters are a-z or space
            return false;
        p++;
    }
    return true;
}

//check if number of max games is a positive number
static bool isValidMaxGame(int num)
{
    if(num < 0)
        return false;
    return true;
}

//check if game length in seconds is a positive number
static bool isValidGameTime(int num)
{
    if(num < 0)
        return false;
    return true;
}

//check if tournament has ended
//TODO: check if we wanna use mapGet function instead (to find tournament ID)
static bool isTournamentEnded(ChessSystem chess, int tournament_id)
{
    assert(chess != NULL);
    ChessTournament current_tournament = (ChessTournament)mapGetFirst(chess->tournaments);
    while(current_tournament->tournament_id != tournament_id){
        current_tournament=mapGetNext(chess->tournaments);
    }
    if(current_tournament->has_ended == true)
        return true;
    return false;
}

//check if 2 players already played together in this tournament.
// If so - game already happened
//TODO: check if we wanna use mapGet function instead (to find tournament ID)
static bool isGameExist (ChessSystem chess, int tournament_id, int first_player, int second_player)
{
    assert(chess != NULL);
    ChessTournament current_tournament = (ChessTournament)mapGetFirst(chess->tournaments);
    while(current_tournament->tournament_id != tournament_id){
        current_tournament=mapGetNext(chess->tournaments);
    }
    ChessGame current_game = (ChessGame)mapGetFirst(current_tournament->games);
    while(current_game != NULL){
        if(current_game->first_player == first_player && current_game->second_player == second_player)
            return true;
        if(current_game->first_player == second_player && current_game->second_player == first_player)
            return true;
        current_game=mapGetNext(current_tournament->games);
    }
    return false;
}

//check if one of the players already played the maximum amount of games allowed in tournament.
//TODO: check if we wanna use mapGet function instead (to find tournament ID)
static bool isMaxExceeded (ChessSystem chess, int tournament_id, int first_player, int second_player)
{
    assert(chess != NULL);
    ChessTournament current_tournament = (ChessTournament)mapGetFirst(chess->tournaments);
    while(current_tournament->tournament_id != tournament_id){
        current_tournament=mapGetNext(chess->tournaments);
    }
    Player current_player = (Player)mapGetFirst(chess->players);
    while(current_player != NULL){
        if(current_player->id == first_player && current_player->num_games > current_tournament->max_games_per_player)
            return true;
        if(current_player->id == second_player && current_player->num_games > current_tournament->max_games_per_player)
            return true;
        current_player=mapGetNext(chess->players);
    }
    return false;
}

//check if player ID is recognized to chess system
static bool isPlayerInSystem(ChessSystem chess, int player_id)
{
    assert(chess != NULL);
    Player current_player = (Player) mapGetFirst(chess->players);
    while(current_player != NULL)
    {
        if(current_player->id == player_id)
            return true;
        current_player=mapGetNext(chess->players);
    }
    return false;
}

//converts map functions results to legitimet chess return values
ChessResult convertMapResultToChessResult(MapResult map_result){
    if(map_result == MAP_NULL_ARGUMENT)
        return CHESS_NULL_ARGUMENT;
    if(map_result == MAP_OUT_OF_MEMORY)
        return CHESS_OUT_OF_MEMORY;
    return CHESS_SUCCESS; //only other option MAP_OUT_OF_MEMORY
}

//creates new tournament, to be added to chess system
//TODO: create map
ChessTournament createTournament(int tournament_id, int max_games_per_player, const char* tournament_location)
{
    ChessTournament Tournament = (ChessTournament) malloc(sizeof(struct chess_tournament_t));
    if(Tournament==NULL)
        return NULL;
    Tournament->tournament_id=tournament_id;
    Tournament->tournament_location=tournament_location;
    Tournament->tournament_winner=-1;
    Tournament->max_games_per_player=max_games_per_player;
    Tournament->has_ended=false;
    Tournament->longest_game_time=-1;
    Tournament->num_valid_time_games=0;
    Tournament->sum_valid_time_games=0;
    Tournament->num_games=0;
    Tournament->num_players=0;

    Map games_map = ;//mapCreate();
    Map players_map ;//= mapCreate();
    Tournament->games=games_map;
    Tournament->players=players_map;
    return Tournament;
}

ChessResult chessAddTournament (ChessSystem chess, int tournament_id, int max_games_per_player, const char* tournament_location)
{
    if(chess == NULL || tournament_location == NULL)
        return CHESS_NULL_ARGUMENT;
    if(!isValidID(tournament_id))
        return CHESS_INVALID_ID;
    if(!isValidLocation(tournament_location))
        return CHESS_INVALID_LOCATION;
    if(!isValidMaxGame(max_games_per_player))
        return CHESS_INVALID_MAX_GAMES;
    if(mapContains(chess->tournaments, tournament_id))
        return CHESS_TOURNAMENT_ALREADY_EXISTS;

    ChessTournament Tournament = createTournament(tournament_id, max_games_per_player, &tournament_location);
    if(Tournament == NULL)
        return CHESS_OUT_OF_MEMORY;
    MapResult map_result = mapPut(chess->tournaments,tournament_id,Tournament);//adding Tournament to chessSystem chess
    return convertMapResultToChessResult(map_result);
}

//creates new game, to be added to chess system
ChessGame createGame(int game_id, int first_player, int second_player, Winner winner, int play_time)
{
    ChessGame Game = (ChessGame) malloc(sizeof(struct chess_game_t));
    if(Game==NULL)
        return NULL;
    Game->game_id = game_id;
    Game->first_player=first_player;
    Game->second_player=second_player;
    Game->game_winner=winner;
    Game->play_time=play_time;
    return Game;
}

//TODO:what to do to game when player is removed?
ChessResult chessAddGame(ChessSystem chess, int tournament_id, int first_player, int second_player, Winner winner, int play_time){
    if(chess == NULL)
        return CHESS_NULL_ARGUMENT;
    if(!isValidID(tournament_id) || !isValidID(first_player) || !isValidID(second_player) || first_player == second_player)
        return CHESS_INVALID_ID;
    if(!mapContains(chess->tournaments, (MapKeyElement) tournament_id))
    return CHESS_TOURNAMENT_NOT_EXIST;
    if(isTournamentEnded(chess,tournament_id))
        return CHESS_TOURNAMENT_ENDED;
    if(isGameExist(chess, tournament_id, first_player, second_player))
        return CHESS_GAME_ALREADY_EXISTS;
    if(!isValidGameTime(play_time))
        return CHESS_INVALID_PLAY_TIME;
    if(isMaxExceeded(chess, tournament_id, first_player, second_player))
        return CHESS_EXCEEDED_GAMES;
    ChessTournament tournament = mapGet(chess->tournaments,(MapKeyElement)tournament_id);
    int game_id;
    if(!tournament){
        game_id = tournament->num_games+1;
    }
    ChessGame Game = createGame(game_id,first_player, second_player, winner, play_time);
    if(Game == NULL)
        return CHESS_OUT_OF_MEMORY;
    MapResult map_result = mapPut((MapKeyElement)tournament_id, Game);//adding Game to chessSystem chess
    return convertMapResultToChessResult(map_result);
}

//TODO: TBD
ChessResult chessEndTournament (ChessSystem chess, int tournament_id){
    if(chess == NULL)
        return CHESS_NULL_ARGUMENT;
    if(!isValidID(tournament_id))
        return CHESS_INVALID_ID;
    if(!mapContains(chess->tournaments, tournament_id))
        return CHESS_TOURNAMENT_NOT_EXIST;
    if(isTournamentEnded(chess,tournament_id))
        return CHESS_TOURNAMENT_ENDED;
};

//TODO: TBD
ChessResult chessRemoveTournament (ChessSystem chess, int tournament_id){
    if(chess == NULL)
        return CHESS_NULL_ARGUMENT;
    if(!isValidID(tournament_id))
        return CHESS_INVALID_ID;
    if(!mapContains(chess->tournaments, tournament_id))
        return CHESS_TOURNAMENT_NOT_EXIST;
};

//TODO: TBD
//does not remove player from data, only flags player as removed
ChessResult chessRemovePlayer(ChessSystem chess, int player_id)
{
    if(chess == NULL)
        return CHESS_NULL_ARGUMENT;
    if(!isPlayerInSystem(chess,player_id))
        return CHESS_PLAYER_NOT_EXIST;

}

//TODO: TBD
ChessResult chessEndTournament (ChessSystem chess, int tournament_id){
    if(chess == NULL)
        return CHESS_NULL_ARGUMENT;
    if(!mapContains(chess->tournaments, tournament_id))
        return CHESS_TOURNAMENT_NOT_EXIST;
    if(isTournamentEnded(chess,tournament_id))
        return CHESS_TOURNAMENT_ENDED;
};

//TODO: TBD
double chessCalculateAveragePlayTime (ChessSystem chess, int player_id, ChessResult* chess_result){
    if(chess == NULL)
        return CHESS_NULL_ARGUMENT;
    if(!isPlayerInSystem(chess,player_id))
        return CHESS_PLAYER_NOT_EXIST;
};

double calculatePlayerLevel(Player player)
{
    int wins = player->num_wins;
    int draws = player->num_draws;
    int losses = player->num_losses;
    double level = 6*wins+2*draws-10*losses;
    return level;
}

int comparePlayers(Player first, Player second)
{
    assert(first && second);
    if(calculatePlayerLevel(first) > calculatePlayerLevel(second))
        return 1;
    if(calculatePlayerLevel(first) < calculatePlayerLevel(second))
        return -1;

    //at this point both players levels are equal, and we order them by dictionary order of their IDs
    if(first->id > second->id)
        return -1;
    if(first->id < second->id)
        return 1;

    //at this point both players levels are equal, and it is the same ID for some reason
    return 0;
}

//TODO: make sure working with file is OK
//TODO: check if we wanna use mapGet function instead (to find tournament ID)
//TODO: where to put comparePlayers?
//check if file is open and writable, otherwise chess_save_failure
ChessResult chessSavePlayersLevels (ChessSystem chess, FILE* file){
    if(chess==NULL)
        return CHESS_NULL_ARGUMENT;
    if(file==NULL)
        return CHESS_NULL_ARGUMENT;
    //make sure working with file is OK
    Player current_player = (Player)mapGetFirst(chess->players);
    while(current_player != NULL)
    {
        assert(chess->num_games > 0); //valid because first player is not null, i.e. games were entered into system
        if(!current_player->has_been_removed)
            fprintf(file,"%d, %.2lf\n",current_player->id, calculatePlayerLevel(current_player)/chess->num_games);
        current_player=mapGetNext(chess->players);
    }
    return CHESS_SUCCESS;
}

//TODO: TBD
ChessResult chessSaveTournamentStatistics (ChessSystem chess, char* path_file){
    if(chess==NULL)
        return CHESS_NULL_ARGUMENT;
    if(path_file==NULL)
        return CHESS_SAVE_FAILURE;
};


