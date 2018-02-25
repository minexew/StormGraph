<?php
header( 'Content-type: application/xhtml+xml' );
header( 'Cache-Control: no-cache, must-revalidate' );

function http_retrieve( $file, $resource )
{
    fputs( $file, "GET " . $resource . " HTTP/1.1\r\n" );
    fputs( $file, "\r\n" );

    $length = 0;

    while ( trim( $line = fgets( $file ) ) != "" )
    {
        list( $key, $value ) = explode( ':', $line, 2 );

        if ( $key == "Content-Length" )
            $length = trim( $value );
    }

    if ( $length > 0 )
        return fread( $file, $length );
    else
        return "";
}

function print_server_status()
{
    $host = '90.179.139.173';
    $port = 80;

    $fp = @fsockopen( $host, $port, $ERROR_NO, $ERROR_STR, (float)0.5 ); 

    if ( $fp )
    {
        $lines = explode( "\n", http_retrieve( $fp, '/status' ) );

        foreach ( $lines as $line )
        {
            list( $key, $value ) = explode( ':', $line, 2 );
            $status[$key] = trim( $value );
        }

        if ( $status['Status'] == 'online' )
        {
            echo '<b>Server is <span style="color: #00AA00">online</span>.</b><br />';
            echo '<i>' . $status['RealmName'] . '</i> has population of <b>' . $status['Population'] . ' players</b>.<br /><br />';
        }
        else if ( $status['Status'] == 'down' )
        {
            echo '<b>Server is <span style="color: #CCCC00">down for maintenance</span>.</b><br />';
            echo 'Reason: <b>' . $status['Reason'] . '</b><br /><br />';
        }
        else
            echo '<b>Server status is <span style="color: #CC0000">unknown</span>.</b><br />';

        $playersOnline = array();

        $lines = explode( "\n", http_retrieve( $fp, '/players' ) );

        foreach ( $lines as $line )
        {
            list( $key, $value ) = explode( ':', $line, 2 );

            if ( $key == 'Player' )
                $playersOnline[] = trim( $value );
        }

        $numPlayers = count( $playersOnline );

        if ( $numPlayers > 0 )
        {
            echo 'Players online: ';

            foreach ( $playersOnline as $i => $player )
            {
                echo '<i>', $player, '</i>';

                if ( $i < $numPlayers - 1 )
                    echo ', ';
            }
        }

        fclose( $fp );
    }
    else
    {
        echo '<b>Server is <span style="color: #FF0000">offline.</span></b>';
        return;
    }

}

echo '<html xmlns="http://www.w3.org/1999/xhtml"><body>';
print_server_status();
echo '</body></html>';

?>