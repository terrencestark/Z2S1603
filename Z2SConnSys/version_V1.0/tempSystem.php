<?php
	#print_r($_POST);
	if (isset($_POST['subType']) && isset($_POST['data1'])){
		$subType = $_POST['subType'];
		$data1 = $_POST['data1'];
		$data2 = $_POST['data2'];
		$data3 = 0x66;  # signal
		
		#socket
		$service_port = 6666;
		$address = '127.0.0.1';
		$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if ($socket === false) {
			echo "socket_create() failed" . "\n";
		}

		$result = socket_connect($socket, $address, $service_port);
		if($result === false) {
			echo "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) . "\n";
		} else {
			#echo "OK \n";
		}
		$in = chr(0x7e) . chr($subType) . chr($data1) . chr($data2) . chr($data3) . chr(0x7e);
		socket_write($socket, $in, strlen($in));
		socket_close($socket);
	}
	
?>
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /> 
		<title><Temprature Management System</title>
	</head>
	<body>
		<table border="1">
			<tr>
				<td><h3>下发指令</h3></td>
				<td></td>
				<td></td>
			</tr>
			
			<tr><form method="post" action="./tempSystem.php">
				<td>
					空调模式：<select name="subType" required id="aim_mode">
						<option value="1">制冷</option>
						<option value="2">制热</option>
					</select>
				</td>
				<td>
					目标温度:<select name="data1" required id="aim_value">
					  <option value="16">16</option>
					  <option value="17">17</option>
					  <option value="18">18</option>
					  <option value="19">19</option>
					  <option value="20">20</option>
					  <option value="21">21</option>
					  <option value="22">22</option>
					  <option value="23">23</option>
					  <option value="24" selected="selected">24</option>
					  <option value="25">25</option>
					  <option value="26" >26</option>
					  <option value="27">27</option>
					  <option value="28">28</option>
					  <option value="29">29</option>
					  <option value="30">30</option>
					</select>
				</td>
				<td>
					<input type="submit" value="打开空调/调温" /> 
				</td>
			</form></tr>
			<tr>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="3" style="display:none;"/>
						<input name="data1" value="0" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="关闭空调" /> 
					</form>
				</td>
				<td>
				</td>
				<td>
				</td>
			</tr>
			<tr><form method="post" action="./tempSystem.php">
				<input name="subType" value="4" style="display:none;"/>
				<td>
					低温阈值：<input type="text" name="data1" value="18" />
				</td>
				<td>
					高温阈值：<input type="text" name="data2" value="30" />
				</td>
				<td>
					<input type="submit" value="警报设定" /> 
				</td>
			</form></tr>
			<tr>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="11" style="display:none;"/>
						<input name="data1" value="1" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="打开LED" /> 
					</form>
				</td>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="11" style="display:none;"/>
						<input name="data1" value="0" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="关闭LED" /> 
					</form>
				</td>
			</tr>
			<tr>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="12" style="display:none;"/>
						<input name="data1" value="1" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="打开门" /> 
					</form>
				</td>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="12" style="display:none;"/>
						<input name="data1" value="0" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="关闭门" /> 
					</form>
				</td>
			</tr>
			<tr>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="13" style="display:none;"/>
						<input name="data1" value="1" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="打开继电器" /> 
					</form>
				</td>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="13" style="display:none;"/>
						<input name="data1" value="0" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="关闭继电器" /> 
					</form>
				</td>
			</tr>
			<tr>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="14" style="display:none;"/>
						<input name="data1" value="1" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="打开安保模式" /> 
					</form>
				</td>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="14" style="display:none;"/>
						<input name="data1" value="0" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="关闭安保模式" /> 
					</form>
				</td>
			</tr>
			<tr>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="15" style="display:none;"/>
						<input name="data1" value="1" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="打开自动室温调节" /> 
					</form>
				</td>
				<td>
					<form method="post" action="./tempSystem.php">
						<input name="subType" value="15" style="display:none;"/>
						<input name="data1" value="0" style="display:none;"/>
						<input name="data2" value="0" style="display:none;"/>
						<input type="submit" value="关闭自动室温调节" /> 
					</form>
				</td>
			</tr>
		</table>
	</body>
</html>