## Ambient-adaptive lamp
Cheap and simple example of ATmega8L-8PU DIP usage.
<p align=center>
<a href="http://www.youtube.com/watch?feature=player_embedded&v=ka44yZxcK2I" target="_blank"><img src="http://img.youtube.com/vi/ka44yZxcK2I/0.jpg" 
alt="project in action" width="240" height="180" border="10" /></a>
</p>

The lamp has 3 microswitches:

- S1 -- ON/OFF
- S2 -- adaptive mode (alter lighting power according to ambient brightness - information from ADC embedded in &#0181;C with the help of a photoresistor)
- S3 -- switch off after 10 seconds

## Scheme:
![scheme](/images/scheme.png)
