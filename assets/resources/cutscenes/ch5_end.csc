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
		<object name="black">
			<texture src="/common/black" />
			<size width="1024" height="768" />
			<position x="0" y="0" z="-100" />
		</object>
		
		<object name="back_1">
			<texture src="ch5_end/back_1" />
			<position z="10" />
		</object>
		<object name="back_3">
			<texture src="ch5_end/back_3" />
			<position z="10" />
		</object>
		
		<object name="gazebo_1">
			<texture src="ch5_end/gazebo_1" />
			<position z="9" />
		</object>
		<object name="gazebo_2">
			<texture src="ch5_end/gazebo_2" />
			<position z="9" />
		</object>
		
		<object name="lipstick">
			<scale value="0.8" />
			<pivot x="26" y="32" />
			<texture src="ch5_end/lipstick" />
			<position z="8" />
		</object>
		
		<object name="delphine_1">
			<texture src="ch5_end/delphine_1" />
			<position z="7" />
		</object>
		<object name="delphine_2">
			<texture src="ch5_end/delphine_2" />
			<position z="7" />
		</object>
		<object name="jessie_1">
			<texture src="ch5_end/jessie_1" />
			<position z="7" />
		</object>
		<object name="jessie_2">
			<texture src="ch5_end/jessie_2" />
			<position z="7" />
		</object>
		<object name="jessie_3">
			<texture src="ch5_end/jessie_3" />
			<position z="7" />
		</object>
		<object name="jessie_4">
			<texture src="ch5_end/jessie_4" />
			<position z="7" />
		</object>
	</objects>

	<actions>
		<streammusic src="/music/cutscenes/ch5_end/sfx_amb_music_cut_scene7" />
		<timer ms="2000">
			<sound src="ch5_end/sound/j_i_did_what_said" />
			<text msg="%杰西: 我做到了我的承诺. 这个季节已经结束了, 而安东夫人一次派对也没参加.%" time="7000">
			<sound src="ch5_end/sound/d_what_you_say" />
			<text msg="%戴尔芬: 你说的没错.%" time="3000">
			<sound src="ch5_end/sound/j_than_youll_be_payin" />
			<text msg="%杰西: 那么你应该按我们约定的, 付给我125美元.%" time="4000">
				<sound src="ch5_end/sound/d_how_do_i_know" />
				<text msg="%戴尔芬: 但是我怎么知道安东夫人的病是否跟你做的事有关系?%" time="4500">
				<text msg="%戴尔芬: 也许, 她只是有些消化不良.%" time="3500">
				<sound src="ch5_end/sound/d_what_kind_of_fool" />
				<text msg="%戴尔芬: 如果我付这么一大笔钱给一个巫毒女巫, 那我不成了傻瓜吗?%" time="5500">
				<text msg="%戴尔芬: 我付给你的25美元, 已经足够了.%" time="3500">
					<sound src="ch5_end/sound/j_you_gave_me_word" />
					<text msg="%杰西: 你向我保证过.%" time="2000">
						<sound src="ch5_end/sound/d_dont_you_use" />
						<text msg="%戴尔芬: 不要用那种语气跟我说话!!!%" time="3000">
						<text msg="%戴尔芬: 现在马上离开, 否则我会控告你!%" time="3000">
							<sound src="ch5_end/sound/j_you_crossed_me" />
							<text msg="%杰西: 你背叛了我, 戴尔芬·拉洛瑞. 你会后悔的...%" time="6000" />
						</text>
						</text>
					</text>
				</text>
				</text>
				</text>
				</text>
			</text>
			</text>
			</text>
		</timer>
		<group>
				<anim object="black" src="ch5_end/alpha_fade_out" />
				<anim object="back_1" src="ch5_end/back_1" />
				<anim object="back_3" src="ch5_end/back_3" />
				<anim object="delphine_1" src="ch5_end/delphine_1" />
				<anim object="delphine_2" src="ch5_end/delphine_2" />
				<anim object="jessie_1" src="ch5_end/jessie_1" />
				<anim object="jessie_2" src="ch5_end/jessie_2" />
				<anim object="jessie_3" src="ch5_end/jessie_3" />
				<anim object="jessie_4" src="ch5_end/jessie_4" />
				<anim object="gazebo_1" src="ch5_end/gazebo_1" />
				<anim object="gazebo_2" src="ch5_end/gazebo_2" />
				<anim object="lipstick" src="ch5_end/lipstick" />
				
				<onFinish>
					<anim object="black" src="ch5_end/alpha_fade_in">
						<end />
					</anim>
				</onFinish>
				
			</group>

	</actions>
</cutscene>
