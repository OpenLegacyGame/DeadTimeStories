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
			<texture src="ch4_mid/back_1" />
			<position z="10" />
		</object>
		<object name="back_2">
			<texture src="ch4_mid/back_2" />
			<position z="10" />
		</object>
		<object name="back_3">
			<texture src="ch4_mid/back_3" />
			<position z="10" />
		</object>
		<object name="back_4">
			<texture src="ch4_mid/back_4" />
			<position z="10" />
		</object>
		
		<object name="boat_1">
			<texture src="ch4_mid/boat_1" />
			<position z="8" />
		</object>
		<object name="boat_2">
			<texture src="ch4_mid/boat_2" />
			<position z="8" />
		</object>
		<object name="mango_falling">
			<texture src="ch4_mid/mango_falling" />
			<position z="8" />
		</object>
		<object name="mango_tree">
			<position y="30" />
			<texture src="ch4_mid/mango_tree" />
			<position z="8" />
		</object>
		<object name="grass">
			<position y="310" />
			<texture src="ch4_mid/grass" />
			<position z="8" />
		</object>
	</objects>
	
	<text msg="%%" time="3000">
	</text>
	
	<actions>
		<streammusic src="\music\cutscenes\ch4_mid\sfx_cutscene5a_amb" />
		
		<timer ms="2000">
			<text msg="%小船慢慢的划了出去, 像是被一只无形的手牵引着...%" time="4000">
				<text msg="%当它经过芒果树下的时候...%" time="4000">
					<text msg="%...一个水果掉了下来.%" time="4000">
						<text msg="%小船像是被施了魔法般的又漂回了你的面前.%" time="4000">
						</text>
					</text>
				</text>
			</text>
		</timer>
		<group>
			<anim object="black" src="ch4_mid/alpha_fade_out" />
			<anim object="back_1" src="ch4_mid/back_1" />
			<anim object="back_2" src="ch4_mid/back_2" />
			<anim object="back_3" src="ch4_mid/back_3" />
			<anim object="back_4" src="ch4_mid/back_4" />
			<anim object="boat_1" src="ch4_mid/boat_1" />
			<anim object="boat_2" src="ch4_mid/boat_2" />
			<anim object="mango_tree" src="ch4_mid/mango_tree" />
			<anim object="mango_falling" src="ch4_mid/mango_falling" />
			<anim object="grass" src="ch4_mid/grass" />
			
			<onFinish>
				<anim object="black" src="ch4_mid/alpha_fade_in">
					<end />
				</anim>
			</onFinish>
			
		</group>
	</actions>
	
</cutscene>
