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
			<texture src="ch2_opening/back_1" />
			<position z="10" />
		</object>
		<object name="back_2">
			<texture src="ch2_opening/back_2" />
			<position z="10" />
			<visible value="0" />
		</object>
		
		<object name="fence">
			<texture src="ch2_opening/fence" />
		</object>
			
		<object name="cloe_side">
			<texture src="ch2_opening/cloe_side" />
		</object>
		<object name="jessie_side">
			<texture src="ch2_opening/jessie_side" />
		</object>
		<object name="cloe_front">
			<texture src="ch2_opening/cloe_front" />
		</object>
		<object name="cloe_front2">
			<texture src="ch2_opening/cloe_front2" />
		</object>
		<object name="jessie_front">
			<texture src="ch2_opening/jessie_front" />
		</object>
	</objects>
	
<!--
<text msg="" time="4000">
</text>
-->
	
	<actions>
		<streammusic src="\music\cutscenes\ch2_opening\sfx_amb_music_cut_scene2" />
		<timer ms="2000">
			<sound src="ch2_opening/sound/j_you_make_that_tea" />
			<text msg="%杰西: 你要在午夜时煮茶, 就像我教你的那样...%" time="3000">
			<text msg="%杰西: ...别再站在那抱怨了.%" time="3000">
			<sound src="ch2_opening/sound/c_i_preciate_that" />			
			<text msg="%克洛: 我明白了, 博德恩女士.%" time="3000">
			<text msg="%克洛: 但是你真的不能为我可怜的侄女贝特西做些什么吗?%" time="4500">
			<text msg="%克洛: 他们把她卖到了可怕的拉洛瑞家.%" time="3500">
			<sound src="ch2_opening/sound/j_igave_you" />
			<text msg="%杰西: 我不是给过你一个圣约翰的保护根吗.%" time="4000">
				<sound src="ch2_opening/sound/c_cant_bring_myself" />
				<text msg="%克洛: 我可不能自己去拿给她.%" time="3000">
				<text msg="%克洛: 他们说戴尔芬·拉洛瑞, 在早餐的时候吃人的肝脏.%" time="7000">
				<sound src="ch2_opening/sound/j_waggin_tongues" />
				<text msg="%杰西: 乱嚼舌头!%" time="3000">
				<sound src="ch2_opening/sound/c_every_servant" />
				<text msg="%克洛: 方圆100英里以内的仆人们都知道她是个恶魔!%" time="4500">
				<text msg="%克洛: 不知道为什么没人对那个女人采取点措施.%" time="4500">
					<sound src="ch2_opening/sound/j_dont_look_at_me" />
					<text msg="%杰西: 别看着我! 我可不是到处诅咒别人的人.%" time="4500">
					<text msg="%杰西: 那是危险的工作. 要付出很大的代价.%" time="4500">
						<sound src="ch2_opening/sound/c_oh_betsey" />
						<text msg="%克洛: 哦, 贝特西. 可怜, 可怜的贝特西...%" time="5000">
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
			<anim object="black" src="ch2_opening/alpha_fade_out" />
			
			<anim object="back_1" src="ch2_opening/back_1" />
			<anim object="back_2" src="ch2_opening/back_2" />
			<anim object="cloe_side" src="ch2_opening/cloe_side" />
			<anim object="jessie_side" src="ch2_opening/jessie_side" />
			<anim object="cloe_front" src="ch2_opening/cloe_front" />
			<anim object="cloe_front2" src="ch2_opening/cloe_front2" />
			<anim object="jessie_front" src="ch2_opening/jessie_front" />
			<anim object="fence" src="ch2_opening/fence" />
			
			<onFinish>
				<anim object="black" src="ch2_opening/alpha_fade_in">
					<end />
				</anim>
			</onFinish>
		</group>
	</actions>
	
</cutscene>
