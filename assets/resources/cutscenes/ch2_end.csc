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
			<position z="-150" />
		</object>
		
		<object name="back_1">
			<position z="-10" />
			<texture src="ch2_end/back_1" />
		</object>
		<object name="back_2">
			<position z="10" />
			<texture src="ch2_end/back_2" />
		</object>
		<object name="fake_back">
			<position z="10" />
			<texture src="ch2_end/fake_back" />
		</object>
		
		<object name="adela">
			<position z="0" />
			<texture src="ch2_end/adelaide" />
		</object>
		
		<object name="jessie_back">
			<position z="0" />
			<texture src="ch2_end/jessie" />
		</object>
		
		<object name="jessie">
			<position z="0" />
			<texture src="ch2_end/jessie_front" />
		</object>
	
	</objects>
	
	<actions>
		<streammusic src="/music/cutscenes/ch2_end/sfx_amb_music_cut_scene3" />
		<timer ms="2000">
			<sound src="ch2_end/sound/ad_is_you_ms_bodeen" />
			<text msg="%阿德莱德: 你是博德恩女士吗?%" time="2000">
				<sound src="ch2_end/sound/j_it_says_so_on_" />
				<text msg="%杰西: 房子上写着呢, 不是吗? 你想要什么?%" time="5000">
				<sound src="ch2_end/sound/j_speakup_now" />
				<text msg="%杰西: 现在说吧.%" time="2000">
					<sound src="ch2_end/sound/ad_my_misterss" />
					<text msg="%阿德莱德: 我的女主人拉洛瑞夫人, 让你马上去一趟.%" time="6000">
					<sound src="ch2_end/sound/j_delphine_lalaurie" />
					<text msg="%杰西: 戴尔芬·拉洛瑞?%" time="3000">
					<sound src="ch2_end/sound/ad_yes_mam" />
					<text msg="%阿德莱德: 是的, 女士. 现在你最好快点.%" time="3000">
					<sound src="ch2_end/sound/j_and_what_if" />
					<text msg="%杰西: 我干嘛要快点?%" time="3000">
					<sound src="ch2_end/sound/ad_oh_no_please" />
					<text msg="%阿德莱德: 哦不! 求你了!%" time="2000">
					<text msg="%阿德莱德: 如果我不按她说的把你带去, 我会有大麻烦的!%" time="6000">
						<sound src="ch2_end/sound/j_hush_child" />
						<text msg="%杰西: 安静点, 孩子!%" time="2000">
						<text msg="%杰西想了想...%" time="2000">
						<sound src="ch2_end/sound/j_i_suppose" />
						<text msg="%杰西: 我猜她有生意要做.%" time="3500">
						<text msg="%杰西: 好吧, 看在钱的份上. 我去拿我的披肩.%" time="4000">
							<sound src="ch2_end/sound/ad_thank_you" />
							<text msg="%阿德莱德: 谢谢你, 博德恩女士!%" time="2500">
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
			<anim object="black" src="ch2_end/alpha_fade_out" />
			
			<anim object="back_1" src="ch2_end/back_1" />
			<anim object="fake_back" src="ch2_end/fake_back" />
			<anim object="back_2" src="ch2_end/back_2" />
			<anim object="adela" src="ch2_end/adela" />
			<anim object="jessie_back" src="ch2_end/jessie_back" />
			<anim object="jessie" src="ch2_end/jessie" />
			
		
			<onFinish>
				<anim object="black" src="ch2_end/alpha_fade_in">
					<end />
				</anim>
			</onFinish>
		</group>
	</actions>
</cutscene>
