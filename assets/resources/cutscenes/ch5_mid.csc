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
			<texture src="ch5_mid/back_1" />
			<position z="10" />
		</object>
		<object name="back_2">
			<texture src="ch5_mid/back_2" />
			<position z="10" />
		</object>
		<object name="back_3">
			<texture src="ch5_mid/back_3" />
			<position z="10" />
		</object>
		<object name="back_4">
			<texture src="ch5_mid/back_4" />
			<position z="0" />
		</object>
		
		<object name="bella_1">
			<texture src="ch5_mid/bella_1" />
			<position z="8" />
		</object>
		<object name="bella_2">
			<texture src="ch5_mid/bella_2" />
			<position z="8" />
		</object>
		<object name="bella_3">
			<texture src="ch5_mid/bella_3" />
			<position z="8" />
		</object>
		
		<object name="jessie_1">
			<texture src="ch5_mid/jessie_1" />
			<position z="8" />
		</object>
		<object name="jessie_2">
			<texture src="ch5_mid/jessie_2" />
			<position z="8" />
		</object>
	</objects>
	
	<text msg="%%" time="3000">
	</text>
	
	<actions>
		<streammusic src="/music/cutscenes/ch5_mid/sfx_amb_music_cut_scene6" />
		<timer ms="2000">
			<sound src="ch5_mid/sound/b_i_cant_do_it" />
			<text msg="%贝拉: 我不能这么做! 我不会再给安东夫人那种不好的药了!%" time="5000">
			<text msg="%贝拉: 她一直对我很好, 而且她现在病得很厉害!%" time="5000">
				<sound src="ch5_mid/sound/j_now_you_listen" />
				<text msg="%杰西: 现在你听着, 贝拉.%" time="2000">
				<text msg="%杰西: 你对安东夫人来说什么都不是, 还不如一只跳蚤!%" time="3500">
				<text msg="%杰西: 而且我们有约定 - 我医治你的妈妈令她解除痛苦.%" time="4500">
				<sound src="ch5_mid/sound/b_i_just_cant" />
				<text msg="%贝拉: 我真的不能!%" time="2000">
				<sound src="ch5_mid/sound/j_guess_ill_have" />
				<text msg="%杰西: 那好吧. 我想我必须把苏珊·怀特斯跟我要的爱情魔药给她了.%" time="6000">
					<sound src="ch5_mid/sound/b_suzanne" />
					<text msg="%贝拉: 哦! 苏珊! 她喜欢我的威廉. 她跟你要魔药了?%" time="6000">
					<sound src="ch5_mid/sound/j_she_did" />
					<text msg="%杰西: 是的. 我告诉她不行. 但是我认为, 也许我们并不是朋友.%" time="6000">
					<sound src="ch5_mid/sound/b_no_please" />
					<text msg="%贝拉: 不, 求你了! 我... 我会按你说的去做...%" time="5000">
						<sound src="ch5_mid/sound/j_thats_better" />
						<text msg="%杰西: 那就最好. 这是药.%" time="3000">
						<text msg="%杰西: 只要在三点的时候往她的茶里放一小撮. 你发誓!%" time="5000">
							<sound src="ch5_mid/sound/b_i_swear" />
							<text msg="%贝拉: 我发誓.%" time="2000">
								<text msg="" time="0" />
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
				<anim object="black" src="ch5_mid/alpha_fade_out" />
				<anim object="back_1" src="ch5_mid/back_1" />
				<anim object="back_2" src="ch5_mid/back_2" />
				<anim object="back_3" src="ch5_mid/back_3" />
				<anim object="back_4" src="ch5_mid/back_4" />
				<anim object="bella_1" src="ch5_mid/bella_1" />
				<anim object="bella_2" src="ch5_mid/bella_2" />
				<anim object="bella_3" src="ch5_mid/bella_3" />
				<anim object="jessie_1" src="ch5_mid/jessie_1" />
				<anim object="jessie_2" src="ch5_mid/jessie_2" />
				
				<onFinish>
					<anim object="black" src="ch5_mid/alpha_fade_in">
						<end />
					</anim>
				</onFinish>
				
			</group>

	</actions>
</cutscene>
