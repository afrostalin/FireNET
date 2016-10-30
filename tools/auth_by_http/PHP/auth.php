<?php
    header("HTTP/1.1 200 OK");
    header("Status: 200");
    header("Content-Type: text/plain");

    if (isset($_POST['login'])) { $login= $_POST['login']; if ($login== '') { unset($login);} }
    if (isset($_POST['password'])) { $password=$_POST['password']; if ($password =='') { unset($password);} }
    
    if (empty($login) or empty($password))
	{
        echo "emptyData";
        exit();
    }

    $email = stripslashes($login);
    $email = htmlspecialchars($login);
    
    $password = stripslashes($password);
    $password = htmlspecialchars($password);

    $email = trim($login);
    $password = trim($password);
	
	// Connect to mysql ( mysql_host, mysql_user, mysql_password )
    $link = mysql_connect('mysql13.000webhost.com', 'a8669788_firenet', 'PgSf8JfC');
	
    if (!$link) 
	{
        echo "mySqlError";
        exit();
    }
	
	// Select users database
    $db_selected = mysql_select_db('a8669788_firenet', $link);
	
    if (!$db_selected)
	{
        echo "mySqlError";
        exit();
    }

    $query = "SELECT * FROM users WHERE login = '" . $login. "'";
    $result = mysql_query($query);
    $row = mysql_fetch_assoc($result);
	
	// If login not found
	if (empty($row))
    {
        echo "loginNotFound";
		mysql_close($link);
        exit();
    }

    if($row['ban'] == "1")	
	{
		echo "userBanned";
		mysql_close($link);
        exit();
	}

	// Check password correct
    if ($row['password']==$password) 
    {
		$uid = $row['uid'];
        echo "uid=$uid";  
        mysql_close($link);
        exit();     
    }
    else
    {
        echo "wrongPassword";
        mysql_close($link);
        exit();    
    }
?>