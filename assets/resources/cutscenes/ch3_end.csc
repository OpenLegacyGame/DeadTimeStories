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
		<object name="back">
			<texture src="ch3_end/back" />
			<position z="10" />
		</object>
		
		<object name="gazebo">
			<texture src="ch3_end/gazebo_1" />
		</object>
		
		<object name="gazebo_2">
			<texture src="ch3_end/gazebo_2" />
		</object>
		
		<object name="jessie_1">
			<texture src="ch3_end/jessie_1" />
			<visible value="0" />
		</object>
		
		<object name="jessie_2">
			<texture src="ch3_end/jessie_2" />
			<visible value="0" />
		</object>
		
		<object name="jessie_3">
			<texture src="ch3_end/jessie_3" />
			<visible value="0" />
		</object>
		
		<object name="jessie_4">
			<texture src="ch3_end/jessie_4" />
			<visible value="0" />
		</object>
		
		<object name="delphine_1">
			<texture src="ch3_end/delphine_1" />
			<visible value="0" />
		</object>
	
		<object name="delphine_2">
			<texture src="ch3_end/delphine_2" />
			<visible value="0" />
		</object>
		
		<object name="delphine_3">
			<texture src="ch3_end/delphine_3" />
			<visible value="0" />
		</object>
		
		<object name="delphine_3">
			<texture src="ch3_end/delphine_3" />
			<visible value="0" />
		</object>
		
		<object name="delphine_4">
			<texture src="ch3_end/delphine_4" />
			<visible value="0" />
		</object>
		
		<object name="delphine_5">
			<texture src="ch3_end/delphine_5" />
			<visible value="0" />
		</object>
			
	</objects>
	<actions>
		<streammusic src="/music/cutscenes/ch3_end/GAZEBO_MUSIC+AMBIENCE_SFX" />
		<timer ms="2000">
			<sound src="ch3_end/sound/d_i_do_thank_SFX" />
			<text msg="%戴尔芬: 非常感谢你能来, 杰西女士. 从现在起我所说的每一件事都只限于我们两个人知道, 好吗?%" time="6000">
			<sound src="ch3_end/sound/j_i'll_take_your_secrets_SFX" />
			<text msg="%杰西: 我会把你的秘密带进坟墓的.%" time="4000">
				<sound src="ch3_end/sound/d_good_now_it's_about_SFX" />
				<text msg="%戴尔芬: 很好. 现在我要说的是关于一位女士, 安东夫人...%" time="4000">
				<text msg="%戴尔芬: 她是从纽约来的, 这种局面完全是她自己造成的...%" time="4000">
				<text msg="%戴尔芬: ...想想吧, 她那些昂贵的巴黎礼服.%" time="4000">
				<sound src="ch3_end/sound/d_its_not_fair_SFX" />
				<text msg="%戴尔芬: 哦, 太不公平了! 为什么她总是受到所有最好派对的邀请?%" time="4000">
					<sound src="ch3_end/sound/j_what_do_you_want_SFX" />
					<text msg="%杰西: 你想让我做什么?%" time="2000">
						<sound src="ch3_end/sound/d_why_i_want_you_to_curse_her_SFX" />
						<text msg="%戴尔芬: 做什么, 我想让你诅咒她! 我才不关心她的死活!%" time="4000">
						<sound src="ch3_end/sound/j_if_the_lady_SFX" />
						<text msg="%杰西: 如果这位女士生病就不能去参加派对了, 对吗?%" time="6000">
						<sound src="ch3_end/sound/d_i_suppose_SFX" />
						<text msg="%戴尔芬: 我想是的. 但是最好能让她一直休息下去.%" time="4500">
							<sound src="ch3_end/sound/j_doing_harm_is_risky_work_SFX" />
							<text msg="%杰西: 害人是有风险的工作. 要付出很大的代价.%" time="2000">
							<text msg="%杰西: 价钱是150美元.%" time="4000">							
							<sound src="ch3_end/sound/d_thats_insane_SFX" />
							<text msg="%戴尔芬: 太贵了!%" time="2000">
							<sound src="ch3_end/sound/j_as_you_like_SFX" />
							<text msg="%杰西: 随你的便.%" time="2000">
								<sound src="ch3_end/sound/d_wait_i_can_pay_you_SFX" />
								<text msg="%戴尔芬: 等等... 我... 我可以现在付你25美元其余的等事情办完再付.%" time="5000">
								<text msg="%戴尔芬: 如果她没有出现在任何一个派对上.%" time="4000">
									<sound src="ch3_end/sound/j_so_it_shall_be_SFX" />
									<text msg="%杰西: 成交.%" time="3000" />
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
				</text>
				</text>
				</text>
				</text>
			</text>
			</text>
		</timer>
		<group>
			<anim object="black" src="ch3_end/alpha_fade_out" />
			
			<anim object="gazebo" src="ch3_end/gazebo" />
			<anim object="gazebo_2" src="ch3_end/gazebo_2" />
			<anim object="delphine_1" src="ch3_end/delphine_1" />
			<anim object="delphine_2" src="ch3_end/delphine_2" />
			<anim object="delphine_3" src="ch3_end/delphine_3" />
			<anim object="delphine_4" src="ch3_end/delphine_4" />
			<anim object="delphine_5" src="ch3_end/delphine_5" />
			<anim object="jessie_1" src="ch3_end/jessie_1" />
			<anim object="jessie_2" src="ch3_end/jessie_2" />
			<anim object="jessie_3" src="ch3_end/jessie_3" />
			<anim object="jessie_4" src="ch3_end/jessie_4" />
			
			<onFinish>
				<anim object="black" src="ch3_end/alpha_fade_in">
					<end />
				</anim>
			</onFinish>
		</group>
	</actions>
	
</cutscene>

