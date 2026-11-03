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
			<position z="-30" />
			<texture src="ch2_garlic/back_1" />
		</object>
		<object name="back_2">
			<position z="-20" />
			<texture src="ch2_garlic/back_2" />
		</object>
		<object name="back_3">
			<position z="-10" />
			<texture src="ch2_garlic/back_3" />
		</object>
		<object name="back_4">
			<position z="0" />
			<texture src="ch2_garlic/back_4" />
		</object>
		
		<object name="garlic">
			<position z="-1" />
			<texture src="ch2_garlic/garlic_falling_1" />
		</object>
	
	</objects>
	
	<actions>
		<timer ms="580">
			<streammusic src="/music/cutscenes/ch2_garlic/sfx_cutscene2a_amb" />
		</timer>
	
		<timer ms="2000">
			<text msg="%你出现了幻觉. 你看到了一阵狂风吹翻了油灯...%" time="3000">
				<text msg="%厨房里着火了, 火势不同寻常的迅速和灼热...%" time="3000">
					<text msg="%房子被怀满仇恨的幽灵烧得颤抖着...%" time="3000">
						<text msg="%幻象逐渐退去... 但是有什么东西被摇的掉了下来.%" time="6000">
						</text>
					</text>
				</text>
			</text>
		</timer>
		
		<group>
			<anim object="black" src="ch2_garlic/alpha_fade_out" />
			
			<anim object="back_1" src="ch2_garlic/back_1" />
			<anim object="back_2" src="ch2_garlic/back_2" />
			<anim object="back_3" src="ch2_garlic/back_3" />
			<anim object="back_4" src="ch2_garlic/back_4" />
			
			<anim object="garlic" src="ch2_garlic/garlic" />
			
			<onFinish>
				<anim object="black" src="ch2_opening/alpha_fade_in">
					<timer ms="2000">
						<end />
					</timer>
				</anim>
			</onFinish>
		</group>
	</actions>
</cutscene>
