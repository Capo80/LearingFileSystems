cmd_/home/capo80/Desktop/LearingFileSystems/BasicFS/Module.symvers := sed 's/ko$$/o/' /home/capo80/Desktop/LearingFileSystems/BasicFS/modules.order | scripts/mod/modpost -m -a   -o /home/capo80/Desktop/LearingFileSystems/BasicFS/Module.symvers -e -i Module.symvers   -T -