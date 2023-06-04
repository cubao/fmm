import json
import os

from fmm import (STMATCH, UBODT, FastMapMatch, FastMapMatchConfig, Network,
                 NetworkGraph, STMATCHConfig)


class MapMatcherConfig:
    def __init__(self, config_json_file):
        if not os.path.exists(config_json_file):
            raise Exception(
                f"File for {config_json_file} is missing.")
        with open(config_json_file) as f:
            data = json.load(f)
        if "model" not in data:
            raise Exception("Model is missing.")
        if "input" not in data:
            raise Exception("Input is missing.")
        if "network" not in data["input"]:
            raise Exception("Network is missing.")
        if "file" not in data["input"]["network"]:
            raise Exception("Network file is missing.")
        self.network_file = str(data["input"]["network"]["file"])

        if not os.path.isfile(self.network_file):
            config_dir = os.path.dirname(os.path.abspath(config_json_file))
            path = os.path.join(config_dir, self.network_file)
            if os.path.isfile(path):
                print(f"update network_file: {self.network_file} -> {path}")
                self.network_file = path

        id = data["input"]["network"].get("id", "id")
        self.network_id = str(id)
        source = data["input"]["network"].get("source", "source")
        self.network_source = str(source)
        target = data["input"]["network"].get("target", "target")
        self.network_target = str(target)
        print(id, source, target, 'oooops')
        if str(data["model"])=="stmatch":
            self.model_tag = "stmatch"
            self.mm_config = STMATCHConfig()
            if "parameters" in data:
                if "k" in data["parameters"]:
                    self.mm_config.k = data["parameters"]["k"]
                if "r" in data["parameters"]:
                    self.mm_config.radius = data["parameters"]["r"]
                if "e" in data["parameters"]:
                    self.mm_config.gps_error = data["parameters"]["e"]
                if "f" in data["parameters"]:
                    self.mm_config.factor = data["parameters"]["f"]
                if "vmax" in data["parameters"]:
                    self.mm_config.vmax = data["parameters"]["vmax"]
        elif (str(data["model"])=="fmm"):
            self.model_tag = "fmm"
            if "ubodt" not in data["input"]:
                raise Exception("Ubodt is missing.")
            if "file" not in data["input"]["ubodt"]:
                raise Exception("Ubodt file is missing.")
            self.ubodt_file = str(data["input"]["ubodt"]["file"])
            self.mm_config = FastMapMatchConfig()
            if "parameters" in data:
                if "k" in data["parameters"]:
                    self.mm_config.k = data["parameters"]["k"]
                if "r" in data["parameters"]:
                    self.mm_config.radius = data["parameters"]["r"]
                if "e" in data["parameters"]:
                    self.mm_config.gps_error = data["parameters"]["e"]
        else:
            raise Exception(
                "Model not found for {} ".format(
                    data["model"]))

class MapMatcher:
    def __init__(self, config_json_file):
        if not os.path.exists(config_json_file):
            raise Exception(
                f"File for {config_json_file} is missing.")
        config = MapMatcherConfig(config_json_file)
        self.network = Network(
            config.network_file,config.network_id,
            config.network_source,config.network_target)
        self.graph = NetworkGraph(
            self.network)
        if config.model_tag=="stmatch":
            self.model = STMATCH(self.network,self.graph)
            self.mm_config = config.mm_config
        elif (config.model_tag=="fmm"):
            self.ubodt = UBODT.read_ubodt_file(
                config.ubodt_file)
            self.model = FastMapMatch(self.network, self.graph, self.ubodt)
            self.mm_config = config.mm_config
        else:
            raise Exception(
                "Model not found for {} ".format(
                    data["model"]))

    def match_wkt(self, wkt):
        return self.model.match_wkt(wkt,self.mm_config)
