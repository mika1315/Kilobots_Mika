## How to execute the program

### For dispersion simulations
```
make clean; make
./dispersion
```

### For the experiment with robots
```
make clean; make hex 
```
select  `dispersion.hex` on the Kilogui software

### To launch dispersion simulations + compute stats on the results

```
mkdir results
make clean && make -j 20
./runKilombo.py -o results -c conf/base.yaml
```

## About dispersion algo
It's based on run and tumble algorithm.

### duration for run state
formula : run_time = $\alpha = \gamma + \phi * \sigma$

(Pmove = $\alpha$)


 - $\gamma$ : offset (or regularization term)
 
   - if $\gamma >= 0, \gamma = 0$ the prob to move when there is no one around
 
   - if $\gamma < 0$, $\gamma$ is a regularization term to dampen the effect of frustration

 - $\phi$ : frustration

   $\phi$ = 1 - d_min / d_optim
- d_min : the distance to the neighbor that is the closest
- d_optim : the ideal distance between one and the other neighbors (It's difficult to find it but the most important thing. You can first start with d_optim = fop (=field of perception = commsRadius))

  - if d_min = d_optim, $\phi = 0$

  - if d_min < d_optim, $\phi > 0$ : frustration

  - if d_min > d_optim, depends whether exploring is wanted or not

 - $\sigma$ : scaling

ensure that $\alpha, \gamma, \phi, \sigma$ are real value

- Pmove can be implemented in various fashion
  - actual prob to move (Pmove = $\alpha$)
  - duration of running <- Here, this is implemented!
    - Pmove is fixed
    - tumble_time is fixed
    - run_time = $f(\alpha)$ or run_time = $f(\alpha)$ +/- N(0, 1) etc...

### duration for tumble state
formula : tumble_time = base_tumble_time +/- N(0, 1)

## About stats to find "good" d_optim

- d_min / d_max
  - d_min : the distance to the neighbor that is the closest
  - d_max : the distance to the neighbor that is the farest away
  
 This value should be close to 1 if they disperse well. However, it doesn't work with the ring arena.

- d_min / fop
  - fop : Field of perception. It is the possible value that could be the distance
  
 The higher this value is, the better they disperse, but it can't be 1. 

***

Hi! :)

I copy-pasted some files from the limmsswarm git repo to launch experiments with your code, and compute dispersion stats based on the results of several runs.
It uses Docker, so that experiments can be launched in a cluster.

To create the docker image:
 - register a new ssh key to access github private repositories from docker. For that you need to create a new ssh key with "ssh-keygen", with an empty passphrase, then give the public key to github: https://docs.github.com/en/developers/overview/managing-deploy-keys
 - use the command: ./createDockerImage.sh PATH/TO/YOUR/PRIVATE/DOCKER/SSH/KEY
 
Then, to launch an experiment using config "conf/base.yaml":
`./runDockerExpe.sh base.yaml`

Example of result:

> ❱ ./runDockerExpe.sh base.yaml
> [...]
> config_filename: conf/base.yaml
> seed: 42
> arenaFileNames: ['arenas/disk.csv', 'arenas/annulus.csv']
> nb_runs: 8
> maxProcesses: 0
> 
> ### arenas/disk.csv ###
> 
> ### arenas/annulus.csv ###
> 
> ##############
> 
> Final stats:  {'mean_mnd_disk': 0.5691137980816813, 'std_mnd_disk': 0.0695850165830106, 'mean_mnd_annulus': 0.6269909291706233, 'std_mnd_annulus': 0.1317864236456948}
> sending incremental file list
> [...]

To have a successful dispersion, we must have:
 - mean_mnd_disk > 0.85
 - mean_mnd_annulus > 0.85
 - std_mnd_disk and std_mnd_annulus as low as possible

The "mnd" is computed by using mean euclidean distance of agents to their neighbors (Mean Neighbors distance). 
Instead of true values of MND, I print out a ratio between 0 and 1:
ratio = ( mnd - min_mnd) / (max_mnd - min_mnd)
where max_mnd is the best possible solution, and min_mnd is the worst possible solution.

:)
