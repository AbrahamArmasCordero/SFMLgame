<html>

	<head>
		<title> User Ranking by Total Pokemons </title>
		
	</head>

	<?php 
		$mysqli = new mysqli("127.0.0.1","root", "2345", "MazeGo");
		if($mysqli ->connect_errno)
		{
			echo 'Fallo en la conexion';
		}
		//echo $mysqli->host_info.'<br>';
	?>

	<body>
		<table border = "2">
			
			<tr><td> User </td><td> Cantidad </td></tr>
			<?php
				$Data = $mysqli->query("SELECT `Players_Data`.`PlayerUser`, SUM(`Pokemon_List`.`Quantity`) AS Total
											FROM `Players_Data`, `Pokemon_List`,`Pokemon_Data`
											WHERE `Players_Data`.`PlayerID`= `Pokemon_List`.`PlayerID_FK`
                                            AND `Pokemon_Data`.`PokemonID` = `Pokemon_List`.`PokemonID_FK`
                                            GROUP BY `Players_Data`.`PlayerUser`
                                            ORDER BY SUM(`Pokemon_List`.`Quantity`) DESC");
				
				//echo $Data->num_rows;
				for($i = 0; $i < $Data->num_rows; $i++)
				{

					$fila = $Data->fetch_assoc();
					
					$var1 = $fila['PlayerUser'];
					$var4 = $fila['Total'];
					
					echo '<tr><td>'.$var1.'</td><td>'.$var4.'</td></tr>';
				}	
			?>
		</table>
		
	</body>
	<?php
	 mysqli_close($mysqli);
	?>
</html>

