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
			<texture src="ch6_end/back_1" />
			<position z="10" />
		</object>
		<object name="back_1_add">
			<texture src="ch6_end/back_1_add" />
			<material src="/materials/quad_tex_add" />
			<position z="9" />
		</object>
		<object name="back_2">
			<texture src="ch6_end/back_2" />
			<position z="10" />
		</object>
		<object name="back_2_add">
			<texture src="ch6_end/back_2_add" />
			<material src="/materials/quad_tex_add" />
			<position z="9" />
		</object>
		<object name="back_3">
			<texture src="ch6_end/back_3" />
			<position z="10" />
		</object>
		<object name="back_4">
			<texture src="ch6_end/back_4" />
			<position z="10" />
		</object>
		<object name="back_4_add">
			<texture src="ch6_end/back_4_add" />
			<material src="/materials/quad_tex_add" />
			<position z="9" />
		</object>
		<object name="back_5">
			<texture src="ch6_end/back_5" />
			<position z="10" />
		</object>
		<object name="back_6">
			<texture src="ch6_end/back_6" />
			<position z="10" />
		</object>
		<object name="back_6_add">
			<texture src="ch6_end/back_6_add" />
			<material src="/materials/quad_tex_add" />
			<position z="9" />
		</object>
		<object name="back_7">
			<texture src="ch6_end/back_7" />
			<position z="10" />
		</object>
		<object name="back_7_add">
			<texture src="ch6_end/back_7_add" />
			<material src="/materials/quad_tex_add" />
			<position z="9" />
		</object>
		<object name="back_8">
			<texture src="ch6_end/back_8" />
			<position z="10" />
		</object>
		<object name="back_8_add">
			<texture src="ch6_end/back_8_add" />
			<material src="/materials/quad_tex_add" />
			<position z="9" />
		</object>
		<object name="back_9">
			<texture src="ch6_end/back_9" />
			<position z="10" />
		</object>
		<object name="back_9_fire">
			<texture src="ch6_end/back_9_fire" />
			<material src="/materials/quad_tex_add" />
			<position x="0" y="6" z="0" />
		</object>
		<object name="back_10">
			<texture src="ch6_end/back_10" />
			<position z="10" />
		</object>
		
		<object name="carriage">
			<texture src="ch6_end/carriage" />
			<position z="8" />
		</object>
		<object name="jessie_1">
			<texture src="ch6_end/jessie_1" />
			<position z="8" />
		</object>
		<object name="jessie_1_add">
			<texture src="ch6_end/jessie_1_add" />
			<material src="/materials/quad_tex_add" />
			<position z="7" />
		</object>
		<object name="jessie_2">
			<texture src="ch6_end/jessie_2" />
			<position z="8" />
		</object>
		<object name="jessie_2_add">
			<texture src="ch6_end/jessie_2_add" />
			<material src="/materials/quad_tex_add" />
			<position z="7" />
		</object>
		<object name="jessie_3">
			<texture src="ch6_end/jessie_3" />
			<position z="8" />
		</object>
		<object name="jessie_4">
			<texture src="ch6_end/jessie_4" />
			<position z="8" />
		</object>
		<object name="jessie_4_add">
			<texture src="ch6_end/jessie_4_add" />
			<material src="/materials/quad_tex_add" />
			<position z="7" />
		</object>
		<object name="jessie_7">
			<texture src="ch6_end/jessie_7" />
			<position z="8" />
		</object>
		<object name="storyteller">
			<texture src="ch6_end/storyteller" />
			<position z="8" />
		</object>
		<object name="two_men">
			<texture src="ch6_end/two_men" />
			<position z="8" />
		</object>
	</objects>
	
	<actions>
		<streammusic src="/music/cutscenes/ch6_end/sfx_OUTRO_amb" />
		<timer ms="2000">
			<sound src="ch6_end/sound/j_damballah_avenge_me" />
			<text msg="%杰西: 丹伯拉, 为我复仇!%" time="3000">
				<timer ms="3000">
					<text msg="%两个晚上之后, 在拉洛瑞的房子外面...%" time="5000">
						<timer ms="4000">
							<sound src="ch6_end/sound/g1_i_ran" />
							<text msg="%绅士甲: 我跑到了阁楼上, 确保所有的仆人都逃出去了.%" time="4000">
							<text msg="%绅士甲: 我在那里看到的真是... 太恐怖了!%" time="3000">
							<sound src="ch6_end/sound/g2_the_stories" />
							<text msg="%绅士乙: 传说是真的! 戴尔芬·拉洛瑞是凶手!%" time="5000">
								<timer ms="4000">
									<sound src="ch6_end/sound/j_run_delphine" />
									<text msg="%杰西: 跑吧, 戴尔芬! 火焰, 它们会找到你的.%" time="7000">
										<sound src="ch6_end/sound/ed_jessie_was_right" />
										<text msg="%爱德华: 杰西是对的. 火焰找到了戴尔芬·拉洛瑞.%" time="5000">
										<text msg="%爱德华: 一个星期后, 一艘客船在纽约港起火, 所有乘客全部遇难.%" time="7000">
											<sound src="ch6_end/sound/ed_as_for_jessie" />
											<text msg="%爱德华: 至于杰西·博德恩, 她的小房子也在10年后付之一炬.%" time="6000">
												<sound src="ch6_end/sound/ed_mambo_marie_warned" />
												<text msg="%爱德华: 曼波·玛丽警告过, 洛阿会执行他自己的判决.%" time="5000">
												<text msg="%爱德华: 就是这样, 我很确定, 你选择了自己的路.%" time="5000">
													<sound src="ch6_end/sound/ed_join_us_next_time" />
													<text msg="%爱德华: 下次再来光临我们的故事宝库吧.%" time="4500">
														<text msg="%爱德华: 我是爱德华·布莱克盖特. 晚安.%" time="4500" />
													</text>
												</text>
												</text>
											</text>
										</text>
										</text>
									</text>
								</timer>
							</text>
							</text>
							</text>
						</timer>
					</text>
				</timer>
			</text>
		</timer>
	
		<group>
				<anim object="black" src="ch6_end/alpha_fade_out" />
				<anim object="back_1" src="ch6_end/back_1" />
				<anim object="back_1_add" src="ch6_end/back_1_add" />
				<anim object="back_2" src="ch6_end/back_2" />
				<anim object="back_2_add" src="ch6_end/back_2_add" />
				<anim object="back_3" src="ch6_end/back_3" />
				<anim object="back_4" src="ch6_end/back_4" />
				<anim object="back_4_add" src="ch6_end/back_4_add" />
				<anim object="back_5" src="ch6_end/back_5" />
				<anim object="back_6" src="ch6_end/back_6" />
				<anim object="back_6_add" src="ch6_end/back_6_add" />
				<anim object="back_7" src="ch6_end/back_7" />
				<anim object="back_7_add" src="ch6_end/back_7_add" />
				<anim object="back_8" src="ch6_end/back_8" />
				<anim object="back_8_add" src="ch6_end/back_8_add" />
				<anim object="back_9" src="ch6_end/back_9" />
				<anim object="back_9_fire" src="ch6_end/back_9_fire" />
				<anim object="back_10" src="ch6_end/back_10" />
				<anim object="carriage" src="ch6_end/carriage" />
				<anim object="jessie_1" src="ch6_end/jessie_1" />
				<anim object="jessie_1_add" src="ch6_end/jessie_1_add" />
				<anim object="jessie_2" src="ch6_end/jessie_2" />
				<anim object="jessie_2_add" src="ch6_end/jessie_2_add" />
				<anim object="jessie_3" src="ch6_end/jessie_3" />
				<anim object="jessie_4" src="ch6_end/jessie_4" />
				<anim object="jessie_4_add" src="ch6_end/jessie_4_add" />
				<anim object="jessie_7" src="ch6_end/jessie_7" />
				<anim object="storyteller" src="ch6_end/storyteller" />
				<anim object="two_men" src="ch6_end/two_men" />

				<onFinish>
					<anim object="black" src="ch6_end/alpha_fade_in">
						<end />
					</anim>
				</onFinish>
				
			</group>

	</actions>
</cutscene>
