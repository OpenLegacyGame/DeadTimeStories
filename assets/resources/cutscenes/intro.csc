<cutscene>
	<objects>
		<object name="line1">
			<texture src="/common/black" />
			<size width="1024" height="96" />
			<position z="-100" />
		</object>
		<object name="line2">
			<texture src="/common/black" />
			<size width="1024" height="96" />
			<position y="672" z="-100" />
		</object>
		
		<object name="moon_shot">
			<texture src="intro/Moon_Shot_1" />
			<scale value="1.95" />
			<position x="-450" y="-300" z="0" />
		</object>
		<object name="leafs_moon">
			<position x="-1600" />
			<texture src="intro/Flying_objects_3" />
		</object>
		<object name="leafs_flying1">
			<pivot x="135" y="63" />
			<texture src="intro/Flying_objects_1" />
			<alpha value="0" />
		</object>
		<object name="leafs_flying2">
			<pivot x="135" y="63" />
			<texture src="intro/Flying_objects_2" />
			<alpha value="0" />
		</object>
		<object name="black">
			<texture src="/common/black" />
			<size width="1024" height="768" />
			<position x="0" y="0" z="-100" />
		</object>
		<object name="back">
			<texture src="intro/Back" />
			<position y="-90" z="10" />
		</object>
		<object name="back_crow">
			<scale value="0.2" />
			<texture src="intro/crow" />
			<position y="-90" z="9.5" />
		</object>
		<object name="back2">
			<texture src="intro/Back_2" />
			<position y="-90" z="9" />
			<alpha value="0" />
		</object>
		<object name="back2_crow">
			<texture src="intro/crow" />
			<position x="640" y="437" z="8.5" />
			<scale value="0.7" />
			<alpha value="0" />
		</object>
		<object name="fog">
			<texture src="intro/Fog" />
			<position x="400" y="570" z="8" />
			<alpha value="0" />
		</object>
		<object name="gates_closed">
			<texture src="intro/Gates_Closed" />
			<position x="0" y="43" z="7" />
		</object>
		<object name="gates_opened">
			<texture src="intro/Gates_Opened" />
			<position x="0" y="43" z="7" />
			<alpha value="0" />
		</object>
		<object name="trees">
			<texture src="intro/Trees" />
			<position x="0" y="43" z="8" />
		</object>
		
		<object name="teller1">
			<texture src="intro/Storyteller_1" />
			<position x="0" y="140" z="3" />
			<alpha value="0" />
		</object>
		<object name="teller2">
			<texture src="intro/Storyteller_2" />
			<position x="0" y="140" z="3" />
			<alpha value="0" />
			<scale value="0.8" />
		</object>
		<object name="teller3">
			<texture src="intro/Storyteller_3" />
			<position x="0" y="140" z="3" />
			<alpha value="0" />
			<scale value="0.8" />
		</object>
		<object name="teller4">
			<texture src="intro/Storyteller_4" />
			<position x="0" y="140" z="3" />
			<alpha value="0" />
			<scale value="0.8" />
		</object>
		<object name="back3">
			<texture src="intro/Back_3" />
			<alpha value="0" />
		</object>
	</objects>
	
	<actions>
		<timer ms="500">
			
			<timer ms="3000">
				<text msg="%爱德华: 啊, 我一直在期待着你...%" time="2800">
				<text msg="%爱德华: 我是爱德华·布莱克盖特. 请进，朋友. 欢迎...%" time="5200">
				</text>
				</text>
			</timer>
			<timer ms="11000"><text msg="%爱德华: 这里是死者的天堂，被称为'来世'.%" time="5500" /></timer>
			<timer ms="16500"><text msg="%爱德华: 你也可以说它是我的杰作.%" time="3000" /></timer>
			<timer ms="19500"><text msg="%爱德华: 我设计了每一尊雕像, 每一朵花... 每一座坟墓.%" time="6500" /></timer>
			<timer ms="26000"><text msg="%爱德华: 那些东西里埋藏着一个故事... 一个奇特的, 绝无仅有的故事.%" time="6300" /></timer>
			<timer ms="32300">
				<text msg="%爱德华: 去找杰西·博德恩, 她是一位来自新奥尔良的巫毒女王.%" time="4000">
				<text msg="%爱德华: 她是一个积极进取的女人,　也许她会越来越成功...%" time="4250">
				<text msg="%爱德华: ...如果她能坚持自己独特的非魔法天性.%" time="4000">
				</text>
				</text>
				</text>
			</timer>
			<timer ms="44550">
				<text msg="%爱德华: 如果你仔细听, 可以听到杰西的故事.%" time="3500">
				<text msg="%爱德华: 嘘! 死者的声音都很轻柔...%" time="3500">
				</text>
				</text>
			</timer>
			
			<streammusic src="/music/cutscenes/intro/sfx_INTRO_amb" />
			<streamsound src="/music/cutscenes/intro/vo_cs1_edward" />
			<anim object="black" src="intro/alpha_fade_out">
				<anim object="moon_shot" src="intro/zoom_out_moon" />
				<anim object="leafs_moon" src="intro/leafs_moon" />
				<timer ms="3000">
					<timer ms="8000">
						<anim object="teller1" src="intro/alpha_fade_in" />
					</timer>
					<group>
						<anim object="gates_opened" src="intro/zoom_in_gates_opened" />
						<anim object="trees" src="intro/zoom_in_trees" />
						<anim object="back" src="intro/zoom_in_back" />
						<anim object="back_crow" src="intro/zoom_in_back_crow" />
						<anim object="gates_closed" src="intro/zoom_in_gates_closed" />
						<anim object="back2" src="intro/back2_fade_in" />
						<anim object="back2_crow" src="intro/back2_crow_fade_in" />
						<onFinish>
							<timer ms="300">
								<anim object="leafs_flying2" src="intro/leafs1" />
								<timer ms="5000">
									<anim object="leafs_flying2" src="intro/leafs1" />
									<timer ms="6000">
										<anim object="leafs_flying1" src="intro/leafs1" />
										<timer ms="500">
											<anim object="leafs_flying2" src="intro/leafs1" />
										</timer>
									</timer>
								</timer>
							</timer>
								
							<group>
								<anim object="leafs_flying1" src="intro/leafs1" />
								<anim object="fog" src="intro/fog" />
								<anim object="teller1" src="intro/teller1" />
								<anim object="teller2" src="intro/teller2" />
								<anim object="teller3" src="intro/teller3" />
								<anim object="teller4" src="intro/teller4" />
								<onFinish>
									<timer ms="2500">
										<anim object="back2" src="intro/zoom_in_back2" />
										<anim object="back2_crow" src="intro/zoom_in_back2_crow" />
										<anim object="teller2" src="intro/zoom_in_teller2" />
										<timer ms="2000">
											<anim object="back3" src="intro/alpha_fade_in">
												<anim object="back3" src="intro/back_last" />
												<timer ms="10000">
													<anim object="black" src="intro/alpha_fade_in">
														<end />
													</anim>
												</timer>
											</anim>
										</timer>
									</timer>
								</onFinish>
							</group>
						</onFinish>
					</group>
				</timer>
			</anim>
		</timer>
	</actions>
	
</cutscene>
