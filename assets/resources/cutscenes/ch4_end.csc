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
			<texture src="ch4_end/back" />
			<position z="10" />
		</object>
		<object name="back_2">
			<texture src="ch4_end/back_2" />
			<position z="10" />
		</object>
		
		<object name="jessie_1">
			<texture src="ch4_end/jessie_1" />
			<position z="8" />
		</object>
		<object name="jessie_2">
			<texture src="ch4_end/jessie_2" />
			<position z="8" />
		</object>
		<object name="jessie_3">
			<texture src="ch4_end/jessie_3" />
			<position z="8" />
		</object>
		<object name="mamba">
			<texture src="ch4_end/mamba" />
			<position z="8" />
		</object>
		<object name="mamba_1">
			<texture src="ch4_end/mamba_1" />
			<position z="8" />
		</object>
		<object name="mamba_2">
			<texture src="ch4_end/mamba_2" />
			<position z="8" />
		</object>
		<object name="mamba_house">
			<texture src="ch4_end/mamba_house" />
			<position z="9" />
		</object>
	</objects>
	
	<text msg="%%" time="3000">
	</text>
	
	<actions>
		<streammusic src="/music/cutscenes/ch4_end/sfx_amb_music_cut_scene5" />
		<timer ms="2000">
			<sound src="ch4_end/sound/mm_one_hundred" />
			<text msg="%曼波·玛丽: 150美元! 的确是一大笔钱.%" time="9000">
			<sound src="ch4_end/sound/j_i_almost" />
			<text msg="%杰西: 我真希望她会拒绝我. 她是个蛇蝎心肠的女人.%" time="5000">
			<sound src="ch4_end/sound/mm_its_an_evil" />
			<text msg="%曼波·玛丽: 这的确是个邪恶的工作. 你可别忘了!%" time="5500">
			<text msg="%曼波·玛丽: 让一个人病上三个月...%" time="4500">
				<sound src="ch4_end/sound/j_i_was_thinkin" />
				<text msg="%杰西: 我想过要用腹蛇诅咒.%" time="3000">
					<sound src="ch4_end/sound/mm_jessie_bodeen" />
					<text msg="%曼波·玛丽: 杰西·博德恩! 千万不要想去诅咒那个女人.%" time="6000">
					<sound src="ch4_end/sound/mm_ain_no_point" />
					<text msg="%曼波·玛丽: 还有, 除非你对那项工作没办法了, 否则千万不要召唤洛阿.%" time="7000">
					<text msg="%曼波·玛丽: 他们可不管你是否应该受到惩罚.%" time="6000">
							<sound src="ch4_end/sound/j_you_are_right" />
							<text msg="%杰西: 你是对的. 那就用药物, 这种伎俩也一样...%" time="5000">
								<sound src="ch4_end/sound/mm_youll_need_someone" />
								<text msg="%曼波·玛丽: 你需要有个人偷偷的潜入她的房子.%" time="5000">
								<sound src="ch4_end/sound/j_i_know_the_maid" />
								<text msg="%杰西: 我认识一个女仆.%" time="2000">
								<sound src="ch4_end/sound/mm_and_i_know" />
								<text msg="%曼波·玛丽: 我知道有一种药.%" time="3500">
								<text msg="%曼波·玛丽: 给我张纸, 我把药的名字写下来...%" time="5500">
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
			<anim object="black" src="ch4_end/alpha_fade_out" />
			<anim object="back" src="ch4_end/back" />
			<anim object="back_2" src="ch4_end/back_2" />
			<anim object="jessie_1" src="ch4_end/jessie_1" />
			<anim object="jessie_2" src="ch4_end/jessie_2" />
			<anim object="jessie_3" src="ch4_end/jessie_3" />
			<anim object="mamba" src="ch4_end/mamba" />
			<anim object="mamba_1" src="ch4_end/mamba_1" />
			<anim object="mamba_2" src="ch4_end/mamba_2" />
			<anim object="mamba_house" src="ch4_end/mamba_house" />
			
			<onFinish>
				<anim object="black" src="ch4_end/alpha_fade_in">
					<end />
				</anim>
			</onFinish>
			
		</group>
	</actions>
	
</cutscene>
