chat "Hossman"
{
	
	#include "teamplay.h"

	//======================================================
	//======================================================

	type "game_enter" //initiated when the bot enters the game
	{
		"The ",4,"? ~This place ain't in my contract.";
		"Not ",1," again! If the $$$ is right you may not lose ... this time.";
		"Make this a good day, cuz it'll be your last.";
		"~Everyone out of the pool.";
		"~You don't seriously think you can beat me.";
		"Time to show you a new level of pain.";
		"I trained in a place just like ",4,".";
		1,", I'm gonna kick your sorry butt into the next county.";
		"I squash cockroaches bigger and scarier than you, ",1,".";
		"Hey there, ho there, hi there!";
		// 4 = Level's Title
		// 1 = random opponent
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Farewell, ",fighter,"s. I got a better paying gig elsewhere.";
		"I guess you can breathe a little easier now that I'm outta here.";
		"Screw this.";
		"Hasta la vista, ",friendname,"s.";
		// 1 = random opponent
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"I have a simple game plan. I shoot you. ~You die. I win. ~You rot.";
		"I'm makin' ",4," my playground. Get off the swings, ",fighter,"s.";
		"To be perfectly honest I could care less who dies, long as I walk out of here.";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"If you want to do my job for me, ",2,", that's fine by me.";
		2," cheats and ",3," sucks. Now that we know, let's go again.";
		"I'm using your ~bones to uphold my reputation.";
		"Waste of time.";
		"Quite the challenge.";
		"~This will not go unavenged.";
		"~This has been truly tragic.";
		// 2 = opponent in first place
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"A ~cool steady hand. The calling card of a professional.";
		"~Did that hurt ya, ",3,"? Bet it did!";
		"Well, we've established that ",3," isn't worth much.";
		"~This is a sucker's job. These ",fighter,"s are all easy marks.";
		"Holey Moley!";
		"Game Over Man!!";
		// 0 = bot name
		// 3 = oppnent in last place
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"~You won't get away with this, ",2,". I don't leave witnesses.";
		"I think my new assignment is redecorating ",2,"'s face.";
		"Ok, I'm ~cool. Just need to frag ",2," more often next time.";
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"~You want to fight me? Me? ~You better get some help, ",0,".";
		"Hey ",0,"! Good idea! Get me mad and you die first.";
		"Well ",0," you're at the top of my hit list ... with a bullet.";
		"~You wanna peice of me? Yer gonna need a bigger spoon!";
		"I'm gonna hurt you, ",0,". I'm gonna hurt you bad.";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Get back here, ",0,", and I'll finish what I started.";
		"That should've killed you! ~Must be a wind change or something.";
		"~You look mighty pretty in blood red.";
		"Lousy, cheap ammo.";
		"Sheee-oooot! I gotta get me a bigger gun than this ~one.";
		"What? ~You're supposed to be dead ",0,"!";
		"~You wearing iron underpants there, ",0,"?";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"~Is this some new form of separation anxiety?";
		"Makes me remember why I gave up drinking.";
		"Ugh. Feels like I slugged down about ",number," shots of ~m0nkey ~killer.";
		"Anybody get the license plate on that elephant?";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"~It's the chance you take when you perform without a net.";
		"Too much beer!";
		"My new technique: a fly-by shooting. Needs a little work.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Heh, I like my coffee hotter than this.";
		"Heh, I like my ",food," hotter than this.";
		"~It's HOT! I need some fresh beer now!";
		"Hoooooweeee! Too much lighter fluid on the grill there.";
		"I guess it's better to burn out than fade away.";
		"Hot? ~You call that hot?";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"~Is this that new special sauce?";
		"Any biscuits with this gravy?";
		"~This is like showerin' in degreaser.";
		"Hey Lucy, this stuff's better'n yer maw's cookin'!";
		"Here I go, hittin' the sauce again.";
		"Just great ... death in a slime mold.";
		"And to think, I used to like lime gelatin.";
		"Ugh. Like swillin' radioactive ",liquid,".";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Yeah sure, never a lifeguard on duty when you need ~one.";
		"Next time, I need to wait an hour after I eat that much ",food,".";
		"No fence. I oughta sue.";
		"No fence. I oughta sue.";
		"I need some beer ... not water!";
		"Great, now I smells like a wet dog.";
		"Stupid place for a swimmmin' pool.";
		"I'm just looking for mermaids.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Now that's the way to dust someone!";
		"Crap!";
		"Doh!";
		"I hate it when that happens.";
		"I'm my own worst enemy.";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Nice punch, ~It's refreshing to have a real competitor.";
		"Get yer slimy paws off o' me, ",0,"!";
		"Last guy that touched me can only count to ~five now.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Sneaky! ~You should talk to me about a job, ",0,"!";
		"Heh. Caught that ~one with my teeth.";
		"~This ain't no campground, ",0,".";
		"Let's settle this outside, face to face.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"My mother fragged me once. Once.";
		"And the friggin' rockets red ... uhhh ... green glare!";
		"Ha, ya missed, ",0,"! 'Almost' only counts in hand grenades and ... oh, puke ...";
		"Stampede!";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Give a kid a weapon and he thinks he's a soldier.";
		0," is still shaking. First kill for you kid?";
		"Ok, ",0,", you I kill for free.";
		"~Two-bit Punk!";
		"~You better have eyes in the back of your head, ",0,".";
		"~You're dog food now, ",0,".";
		"Well I imagine someone's got to win the lottery.";
		"I look better dying than you do shooting, ",0,".";
		"I take my coffee stronger'n you, ",0,".";
		"~You just stepped on the highway to hell, ",fighter,".";
		"I can't believe a ",fighter," like ",0," fragged me!";
		"Hah! That only makes me stronger, ",0,"!";
		"~You won't get to keep that point for long.";
		"~You must be hurting after that, ",0,"!";
		"~They're gonna be picking up pieces of you in the next county.";
		"I'm learning your tricks, ",0,".";
		"Be afraid, ",0,". Be very afraid.";
		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"Hate to admit it, ",0,", but from ~one pro to another, good shot!";
		"Looks like you got a little talent there kid. Use it.";
		"Okay, you win this ~one, ",0,". ~Don't get cocky.";
		"Not bad for an amateur.";
		"And a round of applause goes to ",0,", for a fine performance.";
		"I've seen a lot worse from a lot better. Good shot, ",0,".";
		"Yeah, big hairy deal, ",0,". Ya killed me. So what?";
		"Well, ain't ",0," a ~hero? Come and get your special prize.";
		// 0 = enemy name
	} //end type

	type "death_kamikaze" //initiated when the bot is killed by kamikaze blast
	{
		"~They don't pay me enough to put up with this kinda junk.";
		"Punk!";
		"I am at a lost for words.";
		"I guess you gotta use weapons that match your skill level, ",0,".";
		"No-skill, kamikaze-lovin' twerp.";
		"I bet you post anonymously on message boards too, ",0,".";
		"Real funny, ",0,". ~You're like a one-man BFG.";
		"Loser! ~You wait there so I can kick yer teeth in.";
		"Get that carburetor checked out, ",0,". ~It's back-firing again.";
		"I ain't impressed.";
		"I hate that Commie-Cozy crap.";
		DEATH_KAMIKAZE3;
		// 0 = enemy name
	} //end type 


	//======================================================
	//======================================================

	type "kill_kamikaze" //initiated when the bot kills someone with rail gun
	{
		"Cheap or not, points are points!";
		"I'm selling death and destruction at wholesale prices!";
		"I told you to keep away from me, ",0,". But you wouldn't listen.";
		"I bet they could feel that one all the way to ",place,"!";
		"~You is just like bugs on my windscreen!";
		"Dang. I wuz savin' that for Lucy's birthday!";
		"~You want fries with that?";
		KILL_KAMIKAZE1;
	// 0 = enemy name
	} //end type

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		0,", don't take it personally, but yer makin' my job too easy.";
		"Perfect, ~one slug, one kill.";
		"Turn into the wind, ",fighter,", I wanna hear ya whistle.";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"Great, now my hand is stuck in your skull!";
		"Slap!";
		"Up close and impersonal.";
		"That's the kind of action I like most.";
		"Yeah! Gimme some more of that!";
		"Brutal, but effective.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Geez, ",0,", you make ~one ugly cup of stew.";
		"Spontaneous Frag-bustion!";
		"Here's the beef!";
		"Get off the Road!";
		"Popped you like a Prairie Oyster!";
		"Cleaned and gutted. I'm too good!";
		"Whack! Pay attention ",0,". That's how we do it downtown.";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when player kills self
	{
		"Quick, frag ",0," before the stupid ",fighter," suicides again.";
		"Wow, I guess my reputation precedes me.";
		"Hey moron, AIM, then fire!";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"~You know, ",0,", I really just did it to stop your whining.";
		"Maybe they should call you hamburger instead of ",0,".";
		"~You realize, ",0,", you just keep improving my resume.";
		"Hehehe ... sorry, I just never saw anyone make a face like that ... before I blew it off.";
		"Honestly, ",0,", you look much better dead.";
		"I found your face, ",0,". ~You want it back?";
		"Are you some kind of pacifist, ",0,"?";
		"~You were 'on the phone', right?";
		"That shot belonged in a museum.";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		0," I sure earned my paycheck on you.";
		"Wow! Thats the most excitement ",0," has ever shown me.";
		"Hey, ",0,". At least pretend like you're trying to win.";
		"How many times have you died, ",0,"? There's parts of you everywhere.";
		"~You'll make good maggot food, ",0,". That's something.";
		"~You was brain-dead anyway, ",0,".";
		"Next time, things will not go so easy for you ",0,".";
		"Die with a smile ",0,"!";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		one_liners;
		"I'm on vacation. I just do this to relax.";
		"Wow, is this beginner night?";
		"Look! ",0," is hiding in the corner again!";
		"Hmmm ... so many ways to kill you, ",1,", and so little time.";
		"At least ",1," has a reason to be so bad. What's your excuse, ",0,"?";
		"Hey, ",1,". ~This yellow puddle must be where I nearly got you.";
		"I refuse to have a battle of wits with an unarmed opponent, ",1,".";
		"All the sex appeal of open heart surgery!";
		"~Your mamma's gonna see you on a milk carton, ",0,".";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"BEER! More BEER!";
		"*[Bu-u-u-u-u-u-urp]*";
		"Anybody got a ~six-pack on 'em?";
		"~Don't go tellin' Lucy I was here ... she'll whip my butt.";
		femalebot," better git her ass back in the kitchen and finish making me that chicken pot pie!";
		"~Will someone wake up ",0,". I have to use up my ammo by the expiration date.";
		"So, ",0,". ~Don't you think we oughta smoke ",1,", and then have us a little chat?";
		"~This is my kind of happy hour.";
		"I don't have Lucy's phone number. Try the restroom walls in ",level_title,".";
		// 0 = name of randomly chosen player
		// 1 = bot name
	} //end type
} //end chat
