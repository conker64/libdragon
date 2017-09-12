uint16_t framerate_refresh=0;

// LOAD TEXTURE
sprite_t *read_sprite( const char * const spritename )
{
	
    int fp = dfs_open(spritename);

    if( fp )
    {
        sprite_t *sp = malloc( dfs_size( fp ) );
        dfs_read( sp, 1, dfs_size( fp ), fp );
        dfs_close( fp );
        return sp;
    }
    else
        return 0;
}

// FPS
void update_counter()
{
    framerate_refresh=1;
}

// RANDX
int randx(int min_n, int max_n)
{
    int num1 = MIN( min_n, max_n ) ;
    int num2 = MAX( min_n, max_n ) ;
    int var = num2 - num1 + 1;

    if ( var > RAND_MAX )
        return num1 + rand() * ((( double ) var ) / RAND_MAX );
    else
        return num1 + rand() % var;
}