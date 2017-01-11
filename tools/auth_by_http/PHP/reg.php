<?php
    if (isset($_POST['login'])) { $login = $_POST['login']; if ($login == '') { unset($login);} }
    if (isset($_POST['password'])) { $password=$_POST['password']; if ($password =='') { unset($password);} }
    

    if (empty($login) or empty($password) )
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
    $link = mysql_connect('----------', '---------', '--------');
	
    if (!$link) 
	{
        echo "mySqlError";
        exit();
    }
	
	// Select users database
    $db_selected = mysql_select_db('---------', $link);
	
    if (!$db_selected)
	{
        echo "mySqlError";
        exit();
    }

    $query = "SELECT * FROM users WHERE login = '" . $login . "'";
    $result = mysql_query($query);
    $row = mysql_fetch_assoc($result);
	
	// If login not found - create new user
	if (empty($row))
    {
		$query = "INSERT INTO users (login,password,ban) VALUES ('" . $login ."', '" . $password . "', '0')";
		$result = mysql_query($query);
		
		if ($result == "TRUE")
		{
			echo "registerComplete";
                        mysql_close($link);
                        exit();
		}
		else
		{
			echo "registerFailed";
                        mysql_close($link);
                        exit();
		}
    }
	else
	{
		echo "loginAlredyRegistered";
		mysql_close($link);
        exit();
	}
?>